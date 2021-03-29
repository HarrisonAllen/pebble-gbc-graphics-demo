#include "mario.h"
#include "objects.h"
#include "object_attrs.h"


static uint16_t s_column_to_load;
static uint32_t s_frame;
static uint8_t s_cur_pal;
static MarioGameState s_mario_game_state;
static uint8_t s_mystery_block_palettes[][4] = {
    {0b11011011, 0b11111001, 0b11100100, 0b11000000},
    {0b11011011, 0b11100100, 0b11100100, 0b11000000},
    {0b11011011, 0b11100000, 0b11100100, 0b11000000},
    {0b11011011, 0b11100100, 0b11100100, 0b11000000},
    {0b11011011, 0b11111001, 0b11100100, 0b11000000}
};
static uint8_t s_old_bg_scroll_x, s_old_bg_scroll_y;
static char s_text_buffer[20] = {0};
static uint16_t s_player_score;
static uint8_t s_player_coins;
static uint16_t s_time;
static uint8_t s_player_x_speed;
static short s_player_y_speed;
static bool s_player_moving;
static uint8_t *s_world_map, *s_world_collision_map;
static uint16_t s_player_world_x;
static uint8_t s_player_x, s_player_y;
static uint8_t s_player_jump_start_y;
static bool s_up_pressed;
static bool s_top_bar_show_time;
static MarioJumpState s_player_jump_state;

/* Todo: menu
 *  - Resume
 *  - Restart
 *  - Pokemon
 *  - Save
 *  - Load
 *  - Quit
 */

/* Todo: add Mario
 *  - Down toggles moving mode
 *      - Accelerate up to walking speed
 *      - Accelerate down to stopped
 *  - Select jumps
 *      - While held down, will go up! Until max height, that is.
 *  - Up (short press) changes direction
 *  - Up (long press) menu
 */

/* Todo: add bar at the top
 * Basically the first row of tiles is the stats, then hblank, then draw the screen
 */

static void handle_line_compare(GBC_Graphics *graphics) {
    if (GBC_Graphics_stat_get_line_y_compare(graphics) == 0) {
        s_old_bg_scroll_x = GBC_Graphics_bg_get_scroll_x(graphics);
        s_old_bg_scroll_y = GBC_Graphics_bg_get_scroll_y(graphics);
        GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
        GBC_Graphics_stat_set_line_y_compare(graphics, 8);
    } else {
        GBC_Graphics_bg_set_scroll_pos(graphics, s_old_bg_scroll_x, s_old_bg_scroll_y);
        GBC_Graphics_stat_set_line_y_compare(graphics, 0);
    }
}

static void clear_text_buffer(char *buffer) {
    memset(buffer, 0, sizeof(buffer));
}

static uint8_t convert_char_to_vram_index(char to_convert) {
    if (to_convert >= 'A' && to_convert <= 'Z') {
        return TEXT_START + (to_convert - 'A');
    }
    if (to_convert >= '0' && to_convert <= '9') {
        return NUMBER_START + (to_convert - '0');
    }
    switch (to_convert) {
        case '-': // Dash
            return SYMBOL_START + 0;
        case 'x': // Times symbol
            return SYMBOL_START + 1;
        case '!': // Exclamation point
            return SYMBOL_START + 2;
        case 'c': // Copyright symbol
            return SYMBOL_START + 3;
        case '.': // Period
            return SYMBOL_START + 4;
        case '$': // Coin symbol
            return SYMBOL_START + 5;
        case 'o': // Clock symbol
            return SYMBOL_START + 6;
        case '>': // Right facing pointer
            return SYMBOL_START + 7;
        case '%': // Percent sign
            return SYMBOL_START + 8;
        case ':': // Colon
            return SYMBOL_START + 9;
        case ' ': // Space
            return TEXT_START - 1;
        default:
            return 0;
    }
}

static void write_string_to_background(GBC_Graphics *graphics, uint8_t bg_tile_x, uint8_t bg_tile_y, uint8_t attrs, char *string, uint8_t num_to_write) {
    char char_to_write;
    uint8_t vram_pos;
    if (num_to_write == 0) {
        num_to_write = strlen(string);
    }
    for (uint8_t i = 0; i < num_to_write; i++) {
        char_to_write = string[i];
        vram_pos = convert_char_to_vram_index(char_to_write);
        GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_tile_x + i, bg_tile_y, vram_pos, attrs);
    }
}

static void write_string_to_window(GBC_Graphics *graphics, uint8_t win_tile_x, uint8_t win_tile_y, uint8_t attrs, char *string, uint8_t num_to_write) {
    char char_to_write;
    uint8_t vram_pos;
    if (num_to_write == 0) {
        num_to_write = strlen(string);
    }
    for (uint8_t i = 0; i < num_to_write; i++) {
        char_to_write = string[i];
        vram_pos = convert_char_to_vram_index(char_to_write);
        GBC_Graphics_window_set_tile_and_attrs(graphics, win_tile_x + i, win_tile_y, vram_pos, attrs);
    }
}

static void clear_background(GBC_Graphics *graphics) {
    for (uint8_t i = 0; i < MAP_WIDTH; i++) {
        for (uint8_t j = 0; j < MAP_HEIGHT; j++) {
            GBC_Graphics_bg_set_tile_and_attrs(graphics, i, j, 57, 0);
        }
    }
}

static void clear_window(GBC_Graphics *graphics) {
    for (uint8_t i = 0; i < MAP_WIDTH; i++) {
        for (uint8_t j = 0; j < MAP_HEIGHT; j++) {
            GBC_Graphics_window_set_tile_and_attrs(graphics, i, j, 57, 6);
        }
    }
}

static void update_top_bar(GBC_Graphics *graphics) {
    if (s_top_bar_show_time) {
        char time_buffer[8] = {0};
        clock_copy_time_string(time_buffer, 8);
        clear_text_buffer(s_text_buffer);
        snprintf(s_text_buffer, sizeof(s_text_buffer), "   %8s %3d%%   ", time_buffer, battery_state_service_peek().charge_percent);
        write_string_to_background(graphics, 0, 0, 0 | ATTR_PRIORITY_FLAG, s_text_buffer, 0);
    } else {
        clear_text_buffer(s_text_buffer);
        snprintf(s_text_buffer, sizeof(s_text_buffer), " %5d  x%02d o%03d", s_player_score, s_player_coins, s_time);
        write_string_to_background(graphics, 0, 0, 0 | ATTR_PRIORITY_FLAG, s_text_buffer, 0);
        GBC_Graphics_bg_set_tile_and_attrs(graphics, 7, 0, convert_char_to_vram_index('$'), 1 | ATTR_PRIORITY_FLAG);
    }
}

static void draw_pause_menu(GBC_Graphics *graphics) {
    uint8_t start_x = 6;
    uint8_t start_y = 3;
    write_string_to_window(graphics, start_x, start_y + 0, 6, "CONTINUE", 0);
    write_string_to_window(graphics, start_x, start_y + 2, 6, "RESTART", 0);
    write_string_to_window(graphics, start_x, start_y + 4, 6, "POKEMON", 0);
    write_string_to_window(graphics, start_x, start_y + 6, 6, "SAVE", 0);
    write_string_to_window(graphics, start_x, start_y + 8, 6, "LOAD", 0);
    write_string_to_window(graphics, start_x, start_y + 10, 6, "QUIT", 0);
}

void Mario_initialize(GBC_Graphics *graphics) {
    s_frame = 0;
    s_cur_pal = 0;
    s_player_score = 0;
    s_player_coins = 0;
    s_time = 400;
    s_player_x_speed = 0;
    s_player_y_speed = 0;
    s_player_x = 24;
    // s_player_y = SCREEN_HEIGHT - TILE_HEIGHT * 4 + SPRITE_OFFSET_Y + 1;
    s_player_y = 32 - 1;
    s_player_world_x = s_player_x - 8;

    ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_MARIO_WORLD_MAP);
    size_t res_size = resource_size(handle);
    s_world_map = (uint8_t*)malloc(res_size);
    resource_load(handle, s_world_map, res_size);

    handle = resource_get_handle(RESOURCE_ID_DATA_MARIO_WORLD_COLLISION_MAP);
    res_size = resource_size(handle);
    s_world_collision_map = (uint8_t*)malloc(res_size);
    resource_load(handle, s_world_collision_map, res_size);

    GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_MARIO_TILESHEET, 0, 104, 0, 0);
    GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_MARIO_SPRITESHEET, 0, 20, 0, 1);
    GBC_Graphics_bg_set_scroll_pos(graphics, 0, MAP_HEIGHT * TILE_HEIGHT - SCREEN_HEIGHT);

    clear_background(graphics);
    for (s_column_to_load = 0; s_column_to_load < SCREEN_WIDTH / TILE_WIDTH + 1; s_column_to_load++) {
        Mario_load_column_at_pos(graphics, s_column_to_load, (GBC_Graphics_bg_get_scroll_x(graphics) / TILE_WIDTH) + s_column_to_load, (GBC_Graphics_bg_get_scroll_y(graphics) / TILE_HEIGHT) - 1);
    }
    update_top_bar(graphics);
    
    clear_window(graphics);
    GBC_Graphics_window_set_offset_pos(graphics, 0, 168);
    draw_pause_menu(graphics);

    GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, true);
    GBC_Graphics_set_line_compare_interrupt_callback(graphics, handle_line_compare);
    GBC_Graphics_stat_set_line_y_compare(graphics, 0);

    GBC_Graphics_set_bg_palette(graphics, 0, 0b11011011, 0b11111111, 0b11011111, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 1, 0b11011011, 0b11111001, 0b11100100, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 2, 0b11011011, 0b11101101, 0b11001000, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 3, 0b11011011, 0b11111010, 0b11100100, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 6, 0b11000000, 0b11111111, 0b11111111, 0b11000000);

    for (uint8_t i = 0; i < 40; i++) {
        GBC_Graphics_oam_hide_sprite(graphics, i);
    }
    GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, true);
    GBC_Graphics_oam_set_sprite(graphics, 0, s_player_x, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y, MARIO_STAND, GBC_Graphics_attr_make(4, 1, false, false, false));
    GBC_Graphics_oam_set_sprite(graphics, 1, s_player_x + TILE_WIDTH, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y, MARIO_STAND + 2, GBC_Graphics_attr_make(4, 1, false, false, false));
    GBC_Graphics_set_sprite_palette(graphics, 4, 0b11011011, 0b11111001, 0b11100100, 0b11110000);
    GBC_Graphics_set_sprite_palette(graphics, 5, 0b11011011, 0b11111001, 0b11100100, 0b11001100);

    GBC_Graphics_render(graphics);
}

void Mario_deinitialize(GBC_Graphics *graphics) {
    free(s_world_map);
    free(s_world_collision_map);
}

static uint8_t player_collision(GBC_Graphics *graphics) {
    uint16_t player_left = s_player_world_x + 1;
    uint16_t player_right = player_left + 13;
    uint8_t player_top = s_player_y - 2;
    uint8_t player_bot = player_top - 12; // Go up one up from the player feet

    uint8_t left_block_col = (player_left / 16) % WORLD_LENGTH;
    uint8_t right_block_col = (player_right / 16) % WORLD_LENGTH;
    uint8_t top_block_row = player_top / 16;
    uint8_t bot_block_row = player_bot / 16;

    bool top_left_collision = s_world_collision_map[left_block_col * 2 + (top_block_row < 8 ? 1 : 0)] & (1 << (top_block_row % 8));
    bool top_right_collision = s_world_collision_map[right_block_col * 2  + (top_block_row < 8 ? 1 : 0)] & (1 << (top_block_row % 8));
    bool bot_left_collision = s_world_collision_map[left_block_col * 2 + (bot_block_row < 8 ? 1 : 0)] & (1 << (bot_block_row % 8));
    bool bot_right_collision = s_world_collision_map[right_block_col * 2 + (bot_block_row < 8 ? 1 : 0)] & (1 << (bot_block_row % 8));

    return (((top_left_collision || top_right_collision) * MD_UP)
            | (((!top_right_collision && top_left_collision) || (!bot_right_collision && bot_left_collision)) * MD_LEFT)
            | ((bot_left_collision || bot_right_collision) * MD_DOWN)
            | (((top_right_collision && !top_left_collision) || (bot_right_collision && !bot_left_collision)) * MD_RIGHT));
}

static void play(GBC_Graphics *graphics) {
    s_frame++;
    if (s_frame % 4 == 0) {
        s_cur_pal = (s_cur_pal + 1) % 5;
        uint8_t *pal = s_mystery_block_palettes[s_cur_pal];
        GBC_Graphics_set_bg_palette(graphics, 1, pal[0], pal[1], pal[2], pal[3]);
        GBC_Graphics_render(graphics);
    }
    
    // Update the top bar with our current stats
    if (s_frame % 20 == 0) {
        if (s_time > 0) {
            s_time --;
        }
    }
    if (s_frame % 3 == 0) {
        s_player_score += (s_player_x_speed > 0) ? 10 : 0;
    }
    if (s_frame % 100 == 0) {
        s_player_coins = (s_player_coins + 1) % 100;
    }
    update_top_bar(graphics);

    uint8_t prev_y;
    uint8_t collision_direction;

    // Move vertically
    switch(s_player_jump_state) {
        case MJ_STANDING:
            if (s_up_pressed) {
                s_player_jump_start_y = s_player_y;
                s_player_y_speed = MAX_JUMP_SPEED;
                GBC_Graphics_oam_set_sprite_tile(graphics, 0, MARIO_JUMP);
                GBC_Graphics_oam_set_sprite_tile(graphics, 1, MARIO_JUMP + 2);
                s_player_jump_state = MJ_JUMPING;
            } else {
                collision_direction = player_collision(graphics);
                if (!(collision_direction & MD_DOWN)) {
                    s_player_jump_state = MJ_FALLING;
                    s_player_y_speed = 0;
                }
            }
            break;
        case MJ_JUMPING:
            for (uint8_t i = 0; i < s_player_y_speed; i++) {
                prev_y = s_player_y;
                s_player_y++;
                collision_direction = player_collision(graphics);
                if (collision_direction & MD_UP) {
                    s_player_y = prev_y;
                    s_player_jump_state = MJ_FALLING;
                    break;
                }
            }
            GBC_Graphics_oam_set_sprite_y(graphics, 0, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
            GBC_Graphics_oam_set_sprite_y(graphics, 1, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
            if (!s_up_pressed && s_player_y - s_player_jump_start_y >= MIN_JUMP_HEIGHT) {
                s_player_jump_state = MJ_FALLING;
            }
            if (s_player_y - s_player_jump_start_y >= MAX_JUMP_HEIGHT) {
                s_player_jump_state = MJ_FALLING;
            }
            break;
        case MJ_FALLING:
            s_player_y_speed--;
            for (uint8_t i = 0; i < abs(s_player_y_speed); i++) {
                prev_y = s_player_y;
                s_player_y += s_player_y_speed > 0 ? 1 : -1;

                collision_direction = player_collision(graphics);
                if (collision_direction & MD_DOWN) {
                    s_player_y = prev_y;
                    s_player_jump_state = MJ_STANDING;
                    break;
                } else if (collision_direction & MD_UP) {
                    s_player_y = prev_y;
                    s_player_y_speed = 0;
                    break;
                }
            }
            GBC_Graphics_oam_set_sprite_y(graphics, 0, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
            GBC_Graphics_oam_set_sprite_y(graphics, 1, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
            break;
    }

    // Move horizontally
    if (s_player_moving) {
        if (s_player_x_speed < MAX_X_SPEED) {
            if (s_player_x_speed == 0 || s_frame % 4 == 0) {
                s_player_x_speed++;
            }
        }
    } else {
        if (s_player_x_speed > 0) {
            if (s_player_x_speed == MAX_X_SPEED || s_frame % 4 == 0) {
                s_player_x_speed--;
            }
        }
    }

    if (s_player_jump_state == MJ_STANDING) {
        if (s_player_x_speed > 0) {
            GBC_Graphics_oam_set_sprite_tile(graphics, 0, mario_sprites[1 + (s_frame % 9) / 3]);
            GBC_Graphics_oam_set_sprite_tile(graphics, 1, mario_sprites[1 + (s_frame % 9) / 3] + 2);
        } else {
            GBC_Graphics_oam_set_sprite_tile(graphics, 0, mario_sprites[0]);
            GBC_Graphics_oam_set_sprite_tile(graphics, 1, mario_sprites[0] + 2);
        }
    }

    // Sweep where we're moving to, and load the appropriate columns
    for (uint8_t i = 0; i < s_player_x_speed; i++) {
        uint16_t prev_x = s_player_world_x;
        s_player_world_x++;
        if (player_collision(graphics) & MD_RIGHT) {
            s_player_world_x = prev_x;
            s_player_x_speed = 0;
            break;
        }

        GBC_Graphics_bg_move(graphics, 1, 0);
        uint8_t x = GBC_Graphics_bg_get_scroll_x(graphics);
        if (x % TILE_WIDTH == 0) {
            uint8_t column_x = ((x / TILE_WIDTH) + SCREEN_WIDTH / TILE_WIDTH) % MAP_WIDTH;

            Mario_load_column_at_pos(graphics, s_column_to_load, column_x, (GBC_Graphics_bg_get_scroll_y(graphics) / TILE_HEIGHT) - 1);
            s_column_to_load = (s_column_to_load + 1) % (WORLD_LENGTH * 2);
        }
    }
}

void Mario_step(GBC_Graphics *graphics) {
    switch(s_mario_game_state) {
        case MG_PLAY:
            play(graphics);
            break;
        case MG_PLAY_MENU_MOVING:
            GBC_Graphics_window_move(graphics, 0, -12);
            if (GBC_Graphics_window_get_offset_y(graphics) == 0) {
                s_mario_game_state = MG_PAUSE;
            }
            break;
        case MG_PAUSE:
            break;
        case MG_PAUSE_MENU_MOVING:
            GBC_Graphics_window_move(graphics, 0, 12);
            if (GBC_Graphics_window_get_offset_y(graphics) == SCREEN_HEIGHT) {
                s_mario_game_state = MG_PLAY;
                GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_x, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
                GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_x+TILE_WIDTH, SCREEN_HEIGHT - s_player_y + SPRITE_OFFSET_Y);
            }
            break;
    }
}

void Mario_load_column_at_pos(GBC_Graphics *graphics, uint16_t column, uint8_t bg_tile_x, uint8_t bg_tile_y) {
    // We grab the column from the world map, then draw each of the tiles from the blocks for that column
    const uint8_t *world_map_offset = &s_world_map[((column >> 1) % WORLD_LENGTH) * WORLD_HEIGHT];
    uint8_t column_modifier = column & 1;
    for (uint8_t object_num = 0; object_num < WORLD_HEIGHT; object_num++) {
        const uint8_t *object = object_array[world_map_offset[object_num]];
        const uint8_t *attrs = attr_object_array[world_map_offset[object_num]];
        
        GBC_Graphics_bg_set_tile(graphics, bg_tile_x, bg_tile_y + object_num * 2, object[column_modifier * 2 + 0]);
        GBC_Graphics_bg_set_tile(graphics, bg_tile_x, bg_tile_y + object_num * 2 + 1, object[column_modifier * 2 + 1]);
        GBC_Graphics_bg_set_attrs(graphics, bg_tile_x, bg_tile_y + object_num * 2, attrs[column_modifier * 2 + 0]);
        GBC_Graphics_bg_set_attrs(graphics, bg_tile_x, bg_tile_y + object_num * 2 + 1, attrs[column_modifier * 2 + 1]);
    }
}

void Mario_handle_select(GBC_Graphics *graphics) {
    s_player_moving = !s_player_moving;
}

void Mario_handle_up(GBC_Graphics *graphics, bool up) {
    s_up_pressed = up;
}

void Mario_handle_tap(GBC_Graphics *graphics) {
    s_top_bar_show_time = !s_top_bar_show_time;
}

void Mario_handle_down(GBC_Graphics *graphics) {
    switch(s_mario_game_state) {
        case MG_PLAY:
            GBC_Graphics_oam_hide_sprite(graphics, 0);
            GBC_Graphics_oam_hide_sprite(graphics, 1);
            s_mario_game_state = MG_PLAY_MENU_MOVING;
            break;
        case MG_PAUSE:
            s_mario_game_state = MG_PAUSE_MENU_MOVING;
            break;
        default:
            break;
    }
}

void Mario_handle_focus_lost(GBC_Graphics *graphics) {
    // Pause the game when a notification pops up
    GBC_Graphics_oam_hide_sprite(graphics, 0);
    GBC_Graphics_oam_hide_sprite(graphics, 1);
    s_mario_game_state = MG_PAUSE;
    GBC_Graphics_window_set_offset_y(graphics, 0);
    s_up_pressed = false;
}