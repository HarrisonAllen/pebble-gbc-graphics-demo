#include "pokemon.h"
#include "menu.h"
#include "world.h"
#include "sprites.h"
#include "objects.h"
#include "sprite_decompressor/decompressor.h"
#include "enums.h"

static PlayerDirection s_player_direction = D_DOWN;
static PlayerMode s_player_mode;
static uint8_t s_walk_frame, s_poll_frame, s_battle_frame;
static bool s_select_pressed, s_flip_walk;
static uint16_t s_target_x, s_target_y;
static bool s_can_move;
static uint8_t *s_world_map;
static uint16_t s_player_x, s_player_y;
static uint8_t s_player_sprite_x, s_player_sprite_y;
static Layer *s_background_layer;
static uint8_t *s_player_palette;
static PokemonGameState s_game_state, s_prev_game_state;
static PokemonMenuState s_menu_state;
static PokemonBattleState s_battle_state;
static bool s_save_file_exists;
static uint16_t s_player_pokemon, s_enemy_pokemon;
static uint8_t s_player_pokemon_name[11], s_enemy_pokemon_name[11];
static uint8_t s_step_frame;
static uint8_t s_battle_enemy_scroll_x, s_battle_player_scroll_x;
static uint8_t s_player_pokemon_level, s_enemy_pokemon_level;
static uint16_t s_player_max_pokemon_health, s_enemy_max_pokemon_health;
static uint16_t s_player_pokemon_health, s_enemy_pokemon_health;
static uint8_t s_player_pokemon_damage, s_enemy_pokemon_damage;
static uint8_t s_player_pokemon_attack, s_enemy_pokemon_attack;
static uint8_t s_player_pokemon_defense, s_enemy_pokemon_defense;
static uint8_t s_player_pokemon_exp;
static bool s_clear_dialogue = true;
static uint8_t s_cur_bg_palettes[PALETTE_BANK_SIZE];
static uint16_t s_stats_battles, s_stats_wins, s_stats_losses, s_stats_runs;
static uint8_t s_route_num = 0;
static uint8_t s_player_sprite_choice, s_player_palette_choice;
static uint8_t s_warp_route;
static uint16_t s_warp_x, s_warp_y;

// TODO: 
// - Add in animation tiles
// - Change the hole in the fence to a walkable tile
// - Make cut trees cut
// -- "Would you like to use cut?"
// -- Replace block on map w/ empty square, & walkable
// - Incorporate:
// -- New routes
// -- New tilesheet
// -- Animations
// -- All the other data

static GPoint direction_to_point(PlayerDirection dir) {
    switch (dir) {
        case D_UP:  return GPoint(0, -1);
        case D_LEFT:  return GPoint(-1, 0);
        case D_DOWN:  return GPoint(0, 1);
        case D_RIGHT:  return GPoint(1, 0);
        default: return GPoint(0, 0);
    }
}

void print_array(uint8_t* x, uint16_t len, uint16_t breakpoint) {
    char* print_holder = (char*)malloc(breakpoint*4);
    memset(print_holder, 0, sizeof(print_holder));
    char* hex_array = (char*)malloc(4);
    memset(hex_array, 0, sizeof(hex_array));
    for (uint16_t i = 0; i < len; i++) {
        if (i % breakpoint == 0) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, print_holder);
          memset(print_holder, 0, sizeof(print_holder));
        }
        snprintf(hex_array, 4, "%02x ", x[i]);
        strcat(print_holder, hex_array);
    }
    if (strlen(print_holder) != 0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, print_holder);
    }
    free(print_holder);
    free(hex_array);
}

static uint8_t get_tile_id_from_map(uint8_t *map, uint16_t tile_x, uint16_t tile_y) {
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * map_widths[s_route_num]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return blocks[s_route_num][block * 4 + (tile_x & 1) + (tile_y & 1) * 2];
}

static void load_tiles(GBC_Graphics *graphics, uint8_t bg_root_x, uint8_t bg_root_y, uint16_t tile_root_x, uint16_t tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
  for (uint16_t tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
    for (uint16_t tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
      uint8_t tile = get_tile_id_from_map(s_world_map, tile_x, tile_y);
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, tile_palettes[s_route_num][tile]);
    }
  }
}

static void load_screen(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
  }
  GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
  uint8_t bg_root_x = GBC_Graphics_bg_get_scroll_x(graphics) >> 3;
  uint8_t bg_root_y = GBC_Graphics_bg_get_scroll_y(graphics) >> 3;
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  load_tiles(graphics, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, 18, 18);
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_antialiased(ctx, false);

  GRect rect_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h/2);
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray));
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  // #if defined(PBL_BW)
    // Do some basic dithering
    // LMAO this is unnecessary, the watch can do it already lol, i'll leave it here in case I want to dither something else
    // GBitmap *fb = graphics_capture_frame_buffer(ctx);
    // for (uint8_t y = 0; y < bounds.size.h/2; y++) {
    //   GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    //   for (uint8_t x = 0; x < bounds.size.w; x++) {
    //     uint8_t pixel_color = ((x + (y & 1)) & 1);
    //     uint16_t byte = (x >> 3); // x / 8
    //     uint8_t bit = x & 7; // x % 8
    //     uint8_t *byte_mod = &info.data[byte];
    //     *byte_mod ^= (-pixel_color ^ *byte_mod) & (1 << bit);
    //   }
    // }
    // graphics_release_frame_buffer(ctx, fb);
  // #endif

  rect_bounds = GRect(bounds.origin.x, bounds.origin.y + bounds.size.h/2, bounds.size.w, bounds.size.h/2);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  rect_bounds = GRect(bounds.origin.x, bounds.origin.y + bounds.size.h/2 - 3, bounds.size.w, 6);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  rect_bounds = SCREEN_BOUNDS_SQUARE;
  rect_bounds = GRect(rect_bounds.origin.x - 3, rect_bounds.origin.y - 3, rect_bounds.size.w + 6, rect_bounds.size.h + 6);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, rect_bounds);
  
  rect_bounds = SCREEN_BOUNDS_SQUARE;
  rect_bounds = GRect(rect_bounds.origin.x - 2, rect_bounds.origin.y - 2, rect_bounds.size.w + 4, rect_bounds.size.h + 4);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_rect(ctx, rect_bounds);

  rect_bounds = SCREEN_BOUNDS_SQUARE;
  rect_bounds = GRect(rect_bounds.origin.x - 1, rect_bounds.origin.y - 1, rect_bounds.size.w + 2, rect_bounds.size.h + 2);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, rect_bounds);
}

static void reset_player_palette(GBC_Graphics *graphics) {
  s_player_palette = pokemon_trainer_sprite_palettes[s_player_palette_choice];
  GBC_Graphics_set_sprite_palette(graphics, 0, s_player_palette[0], s_player_palette[1], s_player_palette[2], s_player_palette[3]);
}

static void load_player_sprites(GBC_Graphics *graphics) {
  reset_player_palette(graphics);

  uint16_t spritesheet_offset = pokemon_trainer_sheet_offsets[s_player_sprite_choice] * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET,
    spritesheet_offset, POKEMON_TILES_PER_SPRITE * POKEMON_SPRITES_PER_TRAINER, 2, 3); // offset 2 for effects
}

static void new_player_sprites(GBC_Graphics *graphics) {
  s_player_sprite_choice = rand() % 8;
  s_player_palette_choice = rand() % 6;
  load_player_sprites(graphics);
}

static void move_player_sprites(GBC_Graphics *graphics, short delta_x, short delta_y) {
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_move_sprite(graphics, 2+i, delta_x, delta_y);
  }
}

static void set_player_sprites(GBC_Graphics *graphics, bool walk_sprite, bool x_flip) {
  uint8_t new_tile = pokemon_trainer_sprite_offsets[s_player_direction + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, 2 + i, (new_tile * POKEMON_TILES_PER_SPRITE) + i + 2);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, 2 + i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 2, 3);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 4, 5);
  }
}

static bool load(PokemonSaveData *data) {
  return persist_read_data(SAVE_KEY, data, sizeof(PokemonSaveData)) != E_DOES_NOT_EXIST;
}

static uint8_t get_block_type(uint8_t *map, uint16_t x, uint16_t y) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * map_widths[s_route_num]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return block_types[s_route_num][block];
}

static void load_overworld(GBC_Graphics *graphics) {
  load_screen(graphics);
  
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  load_player_sprites(graphics);

  // Create grass and shadow effect sprites
  uint16_t spritesheet_offset = POKEMON_SPRITELET_GRASS * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset, 2, 0, 3);
  GBC_Graphics_oam_set_sprite(graphics, 0, 0, 0, 0, GBC_Graphics_attr_make(6, 3, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 1, 0, 0, 0, GBC_Graphics_attr_make(6, 3, true, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 6, 0, 0, 1, GBC_Graphics_attr_make(0, 3, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 7, 0, 0, 1, GBC_Graphics_attr_make(0, 3, true, false, false));
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 6, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
  #else
    GBC_Graphics_set_sprite_palette(graphics, 6, 1, 1, 0, 0);
  #endif

  // Create player sprites
  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      GBC_Graphics_oam_set_sprite_pos(graphics, 2 + x + 2 * y, s_player_sprite_x + x * 8, s_player_sprite_y + y * 8 - 4);
      GBC_Graphics_oam_set_sprite_attrs(graphics, 2 + x + 2 * y, GBC_Graphics_attr_make(0, 3, false, false, false));
    }
  }
  set_player_sprites(graphics, false, s_player_direction == D_RIGHT);

  if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
#if defined(PBL_COLOR)
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
#else
    GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
    GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
#endif
  }
}

static void load_resources(GBC_Graphics *graphics) {
  ResHandle handle = resource_get_handle(tilesheet_files[s_route_num]);
  size_t res_size = resource_size(handle);
  uint16_t tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, tilesheet_files[s_route_num], 0, tiles_to_load, 0, 0);
  
  handle = resource_get_handle(map_files[s_route_num]);
  res_size = resource_size(handle);
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
  s_world_map = (uint8_t*)malloc(res_size);
  resource_load(handle, s_world_map, res_size);

  load_overworld(graphics);
}

static void load_game(GBC_Graphics *graphics) {
  s_player_mode = P_STAND;
  s_game_state = PG_PLAY;
  s_walk_frame = 0;
  s_poll_frame = 0;
  s_player_sprite_x = 16 * 4 + SPRITE_OFFSET_X;
  s_player_sprite_y = 16 * 4 + SPRITE_OFFSET_Y;

  load_resources(graphics);
}

static uint32_t combine_4_bytes(uint8_t *array) {
  return array[0] << 24 | array[1] << 16 | array[2] << 8 | array[3];
}

static uint16_t combine_2_bytes(uint8_t *array) {
  return array[0] << 8 | array[1];
}

static void render_pokemon(GBC_Graphics *graphics, GPoint location, uint8_t vram_tile_offset, uint8_t palette) {
  uint8_t i = 0;
  for (uint8_t x = 0; x < 7; x++) {
    for (uint8_t y = 0; y < 7; y++) {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, location.x + x, location.y + y, i+vram_tile_offset, GBC_Graphics_attr_make(palette, 2, false, false, false));
      i++;
    }
  }
}

static void load_palette(GBC_Graphics *graphics, uint16_t pokemon_number, uint8_t palette) {
#if defined(PBL_COLOR)
  ResHandle palette_handle = resource_get_handle(RESOURCE_ID_DATA_POKEMON_PALETTES);
  uint16_t palette_offset = 4 * pokemon_number;
  uint8_t *palette_buffer = (uint8_t*)malloc(4);
  resource_load_byte_range(palette_handle, palette_offset, palette_buffer, 4);
  GBC_Graphics_set_bg_palette(graphics, palette, 
      palette_buffer[0], palette_buffer[2], palette_buffer[1], palette_buffer[3]); // Middle colors are mixed up in data, fix by swapping
  GBC_Graphics_set_sprite_palette(graphics, palette, 
      palette_buffer[0], palette_buffer[2], palette_buffer[1], palette_buffer[3]); // Middle colors are mixed up in data, fix by swapping
  // GBC_Graphics_set_bg_palette_array(graphics, palette, palette_buffer); // If they weren't messed up, just use this
  free(palette_buffer);
#else
  GBC_Graphics_set_bg_palette(graphics, palette, 1, 1, 0, 0);
  GBC_Graphics_set_sprite_palette(graphics, palette, 1, 1, 0, 0);
#endif
}

static void load_pokemon(GBC_Graphics *graphics, uint16_t pokemon_number, bool front, GPoint location, uint8_t vram_tile_offset, uint8_t palette) {
  uint8_t *pokemon_bank = GBC_Graphics_get_vram_bank(graphics, 2) + vram_tile_offset * 16;
  ResHandle data_handle = resource_get_handle(front ? RESOURCE_ID_DATA_POKEMON_FRONT_SPRITE_DATA : RESOURCE_ID_DATA_POKEMON_BACK_SPRITE_DATA);
  uint16_t data_offset = 6 * pokemon_number;
  uint8_t *data_buffer = (uint8_t*)malloc(6);
  resource_load_byte_range(data_handle, data_offset, data_buffer, 6);

  uint32_t sprite_offset = combine_4_bytes(data_buffer);
  uint16_t sprite_size = combine_2_bytes(data_buffer + 4);
  uint32_t sprite_res = front ? RESOURCE_ID_DATA_POKEMON_FRONT_SPRITES : RESOURCE_ID_DATA_POKEMON_BACK_SPRITES;
  load_pokemon_sprite(sprite_res, sprite_offset, sprite_size, pokemon_bank);
  free(data_buffer);

  load_palette(graphics, pokemon_number, palette);

  render_pokemon(graphics, location, vram_tile_offset, palette);
}

static uint8_t lerp_uint8_t(uint8_t start, uint8_t end, float t) {
  return start + (end - start) * t;
}

static uint8_t lerp_color(uint8_t start, uint8_t end, uint8_t index) {
  float t = index / 4.0;
  uint8_t r_1 = ((start >> 4) & 0b11), r_2 = ((end >> 4) & 0b11);
  uint8_t g_1 = ((start >> 2) & 0b11), g_2 = ((end >> 2) & 0b11);
  uint8_t b_1 = ((start) & 0b11), b_2 = ((end) & 0b11);
  uint8_t r = lerp_uint8_t(r_1, r_2, t);
  uint8_t g = lerp_uint8_t(g_1, g_2, t);
  uint8_t b = lerp_uint8_t(b_1, b_2, t);
  return (0b11 << 6) | (r << 4) | (g << 2) | (b);
}

static void lerp_palette(uint8_t *start, uint8_t *end, uint8_t index, uint8_t *output) {
  for (uint8_t c = 0; c < 4; c++) {
    output[c] = lerp_color(start[c], end[c], index);
  }
}

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer) {
  // for (uint8_t i = 0; i < 4; i++) {
  //   lerp_color(0xff, 0x00, i);
  // }

  layer_set_update_proc(background_layer, background_update_proc);
  s_background_layer = background_layer;

  s_game_state = PG_INTRO;
  GBC_Graphics_set_screen_bounds(graphics, SCREEN_BOUNDS_SQUARE);
  GBC_Graphics_window_set_offset_pos(graphics, 0, 168);
  GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, false);
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
  }

  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_MENU_TILESHEET, 0, 121, 0, 1);

  GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
  GBC_Graphics_lcdc_set_bg_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_sprite_layer_enabled(graphics, true);

  set_cursor_pos(0);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);

  draw_blank_rectangle(graphics, GRect(0, 0, 18, 18));
  PokemonSaveData data;
  if (load(&data)) {
    s_save_file_exists = true;

    draw_menu(graphics, GRect(START_MENU_ROOT_X, START_MENU_ROOT_Y, 14, 8), "CONTINUE\n\nNEW GAME\n\nQUIT", false, false);
    set_num_menu_items(3);

    draw_textbox(graphics, GRect(START_INFO_ROOT_X, START_INFO_ROOT_Y, 16, 5), GPoint(1, 1), "Last save:", false);
    struct tm *tick_time = localtime(&data.last_save);
    char text_buffer[20] = {0};
    strftime(text_buffer, sizeof(text_buffer), "%a %b %d", tick_time);
    draw_text_at_location(graphics, GPoint(START_INFO_ROOT_X + 3, START_INFO_ROOT_Y + 2), text_buffer);
    strftime(text_buffer, sizeof(text_buffer), "%r", tick_time);
    draw_text_at_location(graphics, GPoint(START_INFO_ROOT_X + 3, START_INFO_ROOT_Y + 3), text_buffer);
  } else {    
    draw_menu(graphics, GRect(START_MENU_ROOT_X, START_MENU_ROOT_Y, 12, 6), "NEW GAME\n\nQUIT", false, false);
    set_num_menu_items(2);
  }

  // load_game(graphics);
}

void Pokemon_deinitialize(GBC_Graphics *graphics) {
  unload_dialogue();
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
  layer_set_update_proc(s_background_layer, NULL);
}

static void load_blocks_in_direction(GBC_Graphics *graphics, PlayerDirection direction) {
  uint8_t bg_root_x = (GBC_Graphics_bg_get_scroll_x(graphics) >> 3);
  uint8_t bg_root_y = (GBC_Graphics_bg_get_scroll_y(graphics) >> 3);
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  uint16_t num_x_tiles = 0, num_y_tiles = 0;
  switch(direction) {
    case D_UP:
      tile_root_y -= 2;
      bg_root_y -= 2;
      num_x_tiles = 18;
      num_y_tiles = 2;
      break;
    case D_LEFT:
      tile_root_x -= 2;
      bg_root_x -= 2;
      num_x_tiles = 2;
      num_y_tiles = 18;
      break;
    case D_DOWN:
      tile_root_y += 18;
      bg_root_y += 18;
      num_x_tiles = 18;
      num_y_tiles = 2;
      break;
    case D_RIGHT:
      tile_root_x += 18;
      bg_root_x += 18;
      num_x_tiles = 2;
      num_y_tiles = 18;
      break;
  }
  load_tiles(graphics, bg_root_x & 31, bg_root_y & 31, tile_root_x, tile_root_y, num_x_tiles, num_y_tiles);
}

static int check_for_object(uint16_t target_x, uint16_t target_y) {
  uint16_t block_x = target_x >> 4;
  uint16_t block_y = target_y >> 4;

  for (uint16_t i = 0; i < sizeof(objects); i++) {
    if (s_route_num == objects[i][0] && block_x == objects[i][1] && block_y == objects[i][2]) {
      return i;
    }
  }
  return -1;
}

static void lerp_player_palette_to_color(GBC_Graphics *graphics, uint8_t end, uint8_t index) {
  uint8_t palette_holder[4];
  uint8_t end_palette[4] = {end, end, end, end};
  lerp_palette(s_player_palette, end_palette, index, palette_holder);
  GBC_Graphics_set_sprite_palette_array(graphics, 0, palette_holder);
}

static void lerp_bg_palettes_to_color(GBC_Graphics *graphics, uint8_t end, uint8_t index) {
  uint8_t palette_holder[4];
  uint8_t end_palette[4] = {end, end, end, end};
  for (uint8_t i = 0; i < 8; i++) {
    lerp_palette(&s_cur_bg_palettes[i*PALETTE_SIZE], end_palette, index, palette_holder);
    GBC_Graphics_set_bg_palette_array(graphics, i, palette_holder);
  }
}

static void set_player_palette_to_color(GBC_Graphics *graphics, uint8_t color) {
  uint8_t palette[4] = {color, color, color, color};
  GBC_Graphics_set_sprite_palette_array(graphics, 0, palette);
}

static void set_bg_palettes_to_color(GBC_Graphics *graphics, uint8_t color) {
  uint8_t palette[4] = {color, color, color, color};
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, palette);
  }
}

static void play(GBC_Graphics *graphics) {
  if (s_select_pressed && s_player_mode == P_STAND) {
    s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
    s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
    
    s_player_mode = P_WALK;
    s_walk_frame = 0;
    s_flip_walk = !s_flip_walk;
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, false);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, false);

    PokemonSquareInfo block_type = get_block_type(s_world_map, s_target_x+8, s_target_y);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Block at %d, %d yielded type %d", s_target_x+8, s_target_y, block_type);
    if (block_type == OBJECT) {
    
      int object_num = check_for_object(s_target_x+8, s_target_y);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Check at %d, %d yielded object %d", s_target_x+8, s_target_y, object_num);
      if (object_num != -1) {
        PokemonObjectTypes object_type = objects[object_num][3];
        const int16_t *data = &objects[object_num][4];
        switch (object_type) {
          case PO_NONE:
            break;
          case PO_TREE:
          case PO_SIGN:
            load_screen(graphics);
            begin_dialogue(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, data[0], true);
            s_prev_game_state = s_game_state;
            s_game_state = PG_DIALOGUE;
            s_player_mode = P_STAND;
            s_can_move = false;
            break;
          case PO_WARP:{
            s_warp_route = data[0];
            s_warp_x = data[1];
            s_warp_y = data[2];
            if (s_player_direction == D_UP) {
              s_player_mode = P_WARP_WALK;
            } else {
              s_player_mode = P_WARP;
            }
            s_can_move = true;
          } break;
          default:
            break;
        }
      }
    } else {
      if ((block_type == CLIFF_S && s_player_direction == D_DOWN)
          || (block_type == CLIFF_W && s_player_direction == D_LEFT)
          || (block_type == CLIFF_E && s_player_direction == D_RIGHT)) {
        s_target_x += direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
        s_target_y += direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
        s_player_mode = P_JUMP;
        s_can_move = true;
      } else {
        s_can_move = (block_type == WALK || block_type == GRASS);
      }
      // if block type == cliff and direction == (based on cliff)
      // target pos = + some more, mode = jump

      if (!s_can_move) {
        s_target_x = s_player_x;
        s_target_y = s_player_y;
      } else {
        load_blocks_in_direction(graphics, s_player_direction);
      #if defined(PBL_BW)
        GBC_Graphics_oam_hide_sprite(graphics, 0);
        GBC_Graphics_oam_hide_sprite(graphics, 1);
      #endif
      }
    }
  }
  switch(s_player_mode) {
    case P_STAND:
      // if (s_poll_frame == 0) { // Reduce polling rate to reduce battery drain
      //   accel_service_peek(&accel);
      //   if (abs(accel.x) > abs(accel.y)) {
      //     if (accel.x < -150) {
      //       s_player_direction = D_LEFT;
      //     } else if (accel.x > 150) {
      //       s_player_direction = D_RIGHT;
      //     }
      //   } else {
      //     if (accel.y < -150) {
      //       s_player_direction = D_DOWN;
      //     } else if (accel.y > 150) {
      //       s_player_direction = D_UP;
      //     }
      //   }
      //   if (old_direction != s_player_direction) {
      //     set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
      //   }
      // }
      break;
    case P_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS) { // grass
        switch(s_walk_frame) {
          case 2:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
            break;
          case 5:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x - 1, s_player_sprite_y + 5);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 9, s_player_sprite_y + 5);
            break;
          case 7:
          #if defined(PBL_COLOR)
            GBC_Graphics_oam_hide_sprite(graphics, 0);
            GBC_Graphics_oam_hide_sprite(graphics, 1);
          #else
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
          #endif
            if (s_can_move && (rand() % WILD_ODDS == 0)) {
              load_screen(graphics);
              s_game_state = PG_BATTLE;
              s_battle_state = PB_FLASH;
              s_battle_frame = 0;
            }
            break;
          default:
            break;
        }
      }
      switch(s_walk_frame) {
        case 7:
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
          break;
        case 0:
        case 6:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 2:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      s_walk_frame++;
      break;
    case P_JUMP:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS && s_walk_frame == 15) { // grass
        if (s_can_move && (rand() % WILD_ODDS == 0)) {
          load_screen(graphics);
          s_game_state = PG_BATTLE;
          s_battle_state = PB_FLASH;
          s_battle_frame = 0;
        }
      }
      switch(s_walk_frame) {
        case 15:
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
          GBC_Graphics_oam_hide_sprite(graphics, 6);
          GBC_Graphics_oam_hide_sprite(graphics, 7);
          break;
        case 0:
          GBC_Graphics_oam_set_sprite_pos(graphics, 6, s_player_sprite_x, s_player_sprite_y + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 7, s_player_sprite_x + 8, s_player_sprite_y + 8);
        case 6:
        case 8:
        case 14:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 10:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        case 7:
          load_blocks_in_direction(graphics, s_player_direction);
          break;
        default:
          break;
      }
      switch(s_walk_frame) {
        case 0:
          move_player_sprites(graphics, 0, -3);
          break;
        case 1:
          move_player_sprites(graphics, 0, -2);
          break;
        case 2:
          move_player_sprites(graphics, 0, -2);
          break;
        case 3:
          move_player_sprites(graphics, 0, -1);
          break;
        case 4:
          move_player_sprites(graphics, 0, -1);
          break;
        case 5:
          move_player_sprites(graphics, 0, -1);
          break;
        case 9:
          move_player_sprites(graphics, 0, 1);
          break;
        case 10:
          move_player_sprites(graphics, 0, 1);
          break;
        case 11:
          move_player_sprites(graphics, 0, 1);
          break;
        case 12:
          move_player_sprites(graphics, 0, 2);
          break;
        case 13:
          move_player_sprites(graphics, 0, 2);
          break;
        case 14:
          move_player_sprites(graphics, 0, 3);
          break;
        default:
          break;
      }
      s_walk_frame++;
      break;
    case P_WARP_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
      }
      switch(s_walk_frame) {
        case 7:
          if (s_player_direction == D_DOWN) {
            s_player_mode = P_STAND;
          } else {
            s_player_mode = P_WARP;
            s_walk_frame = 0;
          }
          break;
        case 0:
        case 6:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 2:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      if (s_player_mode == P_WARP_WALK) {
        s_walk_frame++;
      }
      break;
    case P_WARP:
      if (s_walk_frame < 4) {
        if (s_walk_frame == 0) {
          GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
        }
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, s_walk_frame);
        lerp_player_palette_to_color(graphics, 0b11111111, s_walk_frame);
      #else
        if (s_walk_frame == 3) {
          set_bg_palettes_to_color(graphics, 1);      
          set_player_palette_to_color(graphics, 1);
        }
      #endif
      } else {
        if (s_walk_frame == 4) {  
          s_route_num = s_warp_route;
          s_player_x = s_warp_x;
          s_player_y = s_warp_y;
          s_target_x = s_warp_x;
          s_target_y = s_warp_y;
          load_resources(graphics);
          load_blocks_in_direction(graphics, s_player_direction);
          GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
        #if defined(PBL_BW)
          set_bg_palettes_to_color(graphics, 1);
        #endif
        }
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, 7 - s_walk_frame);
        lerp_player_palette_to_color(graphics, 0b11111111, 7 - s_walk_frame);
      #else
        if (s_walk_frame == 5) {
          for (uint8_t i = 0; i < 8; i++) {
            GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
          }
        }
      #endif
      }
      if (s_player_mode == P_WARP) {
        s_walk_frame++;
      }
      if (s_walk_frame == 7) {
        if (s_player_direction == D_DOWN) {
          s_player_mode = P_WARP_WALK;
          s_walk_frame = 0;
        } else {
          s_player_mode = P_STAND;
        }
      }
      break;
    default:
      break;
  }
}

static void draw_menu_info(GBC_Graphics *graphics) {
  switch (get_cursor_pos()) {
    case 0:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Close this\n\nmenu", true);
      break;
    case 1:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "View time &\n\nPebble info", true);
      break;
    case 2:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "View game\n\nstatistics", true);
      break;
    case 3:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Save your\n\nprogress", true);
      break;
    case 4:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Quit this\n\ndemo app", true);
      break;
    default:
      break;
  }
}

static void draw_pause_menu(GBC_Graphics *graphics) {
  load_screen(graphics);
  draw_menu(graphics, GRect(MENU_ROOT_X, MENU_ROOT_Y, 9, NUM_MENU_ITEMS * 2 + 2), "RESUME\n\nPEBBLE\n\nSTATS\n\nSAVE\n\nQUIT", false, false);
  set_num_menu_items(NUM_MENU_ITEMS);
  draw_menu_info(graphics);
}

static void handle_battle_scroll_interrupt(GBC_Graphics *graphics) {
  if (GBC_Graphics_stat_get_line_y_compare(graphics) == 0) {
    GBC_Graphics_bg_set_scroll_x(graphics, s_battle_enemy_scroll_x);
    GBC_Graphics_stat_set_line_y_compare(graphics, 64);
  } else if (GBC_Graphics_stat_get_line_y_compare(graphics) == 64) {
    GBC_Graphics_bg_set_scroll_x(graphics, s_battle_player_scroll_x);
    GBC_Graphics_stat_set_line_y_compare(graphics, 96);
  } else if (GBC_Graphics_stat_get_line_y_compare(graphics) == 96) {
    GBC_Graphics_bg_set_scroll_x(graphics, 0);
    GBC_Graphics_stat_set_line_y_compare(graphics, 0);
  } 
}

static uint16_t calculate_health(uint8_t level) {
  uint8_t base = rand()%100 + 50;
  return ((base + 50) * level) / 50 + 10;
}

static uint16_t calculate_damage(uint8_t level, uint8_t attack, uint8_t defense) {
  uint8_t crit = (rand()%20==0) ? 1.5 : 1;
  return (uint16_t)(((((2.0 * level / 5.0 + 2.0) * attack * 80.0 / defense) / 50) + 2) * (rand()%16 + 85) / 100) * crit;
}

static void generate_pokemon_stats() {
  s_player_pokemon_level = rand()%100 + 1;
  int enemy_level = s_player_pokemon_level + rand()%17 - 8;
  if (enemy_level < 1) {
    enemy_level = 1;
  } else if (enemy_level > 100) {
    enemy_level = 100;
  }
  s_enemy_pokemon_level = enemy_level;
  
  s_player_max_pokemon_health = calculate_health(s_player_pokemon_level);
  s_enemy_max_pokemon_health = calculate_health(s_enemy_pokemon_level);
  s_player_pokemon_health = s_player_max_pokemon_health;
  s_enemy_pokemon_health = s_enemy_max_pokemon_health;
  s_player_pokemon_attack = (rand()%15+5)*(s_player_pokemon_level/10.0+1);
  s_enemy_pokemon_attack = (rand()%15+5)*(s_player_pokemon_level/10.0+1);
  s_player_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1);
  s_enemy_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1);

  s_player_pokemon_exp = rand()%65;
}

static void battle(GBC_Graphics *graphics) {
  switch(s_battle_state) {
    case PB_FLASH: {
      uint8_t frame_mod = s_battle_frame % 8;
    #if defined(PBL_COLOR)
      if (frame_mod == 0 || frame_mod == 4) {
        lerp_bg_palettes_to_color(graphics, 0b11000000, 0);
      } else if (frame_mod == 1 || frame_mod == 3) {
        lerp_bg_palettes_to_color(graphics, 0b11000000, 2);
      } else if (frame_mod == 2) {
        lerp_bg_palettes_to_color(graphics, 0b11000000, 4);
      } else if (frame_mod == 5 || frame_mod == 7) {
        lerp_bg_palettes_to_color(graphics, 0b11111111, 2);
      } else if (frame_mod == 6) {
        lerp_bg_palettes_to_color(graphics, 0b11111111, 4);
      }
      /* //Old version, slower, but goes through full spectrum
      if (s_battle_frame % 20 < 5) {
        lerp_bg_palettes_to_color(graphics, 0b11000000, s_battle_frame % 5);
      } else if (s_battle_frame % 20 < 10) {
        lerp_bg_palettes_to_color(graphics, 0b11000000, 4 - s_battle_frame % 5);
      } else if (s_battle_frame % 20 < 15) {
        lerp_bg_palettes_to_color(graphics, 0b11111111, s_battle_frame % 5);
      } else {
        lerp_bg_palettes_to_color(graphics, 0b11111111, 4 - s_battle_frame % 5);
      } */
    #else
      if (frame_mod == 0 || frame_mod == 4) {
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
        }
      } else if (frame_mod == 2) {
        set_bg_palettes_to_color(graphics, 0);
      } else if (frame_mod == 6) {
        set_bg_palettes_to_color(graphics, 1);
      }
      /* //Old version, slower, matches old slow color transition
      if (s_battle_frame % 20 == 5) {
        set_bg_palettes_to_color(graphics, 0);
      } else if (s_battle_frame % 20 == 10) {
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, bg_palettes[i]);
        }
      } else if (s_battle_frame % 20 == 15) {
        set_bg_palettes_to_color(graphics, 1);
      } else if (s_battle_frame % 20 == 0) {
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, bg_palettes[i]);
        }
      } */
    #endif
      s_battle_frame++;
      if (s_battle_frame == 24) {
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
        }
        s_battle_state = PB_WIPE;
        s_battle_frame = 0;
      }
    } break;
    case PB_WIPE:
      if (s_battle_frame <= 40) {
        for (uint8_t i = 0; i < 14; i++) {
          GBC_Graphics_bg_set_tile_and_attrs(graphics, rand()%18, rand()%18, 0, GBC_Graphics_attr_make(7, 1, 0, 0, 1));
        }
        s_battle_frame++;
      } else {
        for (uint8_t x = 0; x < 18; x++) {
          for (uint8_t y = 0; y < 18; y++) {
            GBC_Graphics_bg_set_tile_and_attrs(graphics, x, y, 0, GBC_Graphics_attr_make(7, 1, 0, 0, 1));
          }
        }
        s_battle_frame = 0;
        s_battle_state = PB_LOAD;
      }
      break;
    case PB_LOAD:
      // Load in the pokemon
      s_battle_frame = 0;
      s_battle_state = PB_SLIDE;
      for (uint8_t x = 0; x < 32; x++) {
        for (uint8_t y = 0; y < 32; y++) {
          GBC_Graphics_bg_set_tile_and_attrs(graphics, x, y, 0, GBC_Graphics_attr_make(0, 1, false, false, false));
        }
      }
      s_player_pokemon = rand() % 251 + 1;
      s_enemy_pokemon = rand() % 251 + 1;
      load_pokemon(graphics, s_player_pokemon, false, PLAYER_LOCATION, TILE_PLAYER_OFFSET, 1);
      load_pokemon(graphics, s_enemy_pokemon, true, ENEMY_LOCATION, TILE_ENEMY_OFFSET, 2);
    #if defined(PBL_COLOR)
      GBC_Graphics_set_bg_palette(graphics, 1, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
      GBC_Graphics_set_sprite_palette(graphics, 1, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
      GBC_Graphics_set_bg_palette(graphics, 2, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
      GBC_Graphics_set_sprite_palette(graphics, 2, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
    #endif
      for (uint8_t y = 0; y < 3; y++) {
        for (uint8_t x = 0; x < 7; x++) {
          GBC_Graphics_bg_move_tile(graphics, 0+x, 5+y, 0+x, 20+y, true);
          GBC_Graphics_oam_set_sprite(graphics, x+y*7, 0*8+x*8+8 + 144, 5*8+y*8+16, y+x*7, GBC_Graphics_attr_make(1, 2, false, false, false));
        }
      }
      ResHandle data_handle = resource_get_handle(RESOURCE_ID_DATA_POKEMON_NAMES);
      uint16_t data_offset = 10 * s_player_pokemon;
      resource_load_byte_range(data_handle, data_offset, s_player_pokemon_name, 10);
      data_offset = 10 * s_enemy_pokemon;
      resource_load_byte_range(data_handle, data_offset, s_enemy_pokemon_name, 10);

      draw_menu_rectangle(graphics, DIALOGUE_BOUNDS);

      generate_pokemon_stats();
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Player Pokemon:\n\tName: %s\n\tLevel: %d\n\tHP: %d\n\tAttack: %d\n\tDefense: %d\n\t",
              s_player_pokemon_name, s_player_pokemon_level, s_player_pokemon_health, s_player_pokemon_attack, s_player_pokemon_defense);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Enemy Pokemon:\n\tName: %s\n\tLevel: %d\n\tHP: %d\n\tAttack: %d\n\tDefense: %d\n\t",
              s_enemy_pokemon_name, s_enemy_pokemon_level, s_enemy_pokemon_health, s_enemy_pokemon_attack, s_enemy_pokemon_defense);

      // And set up for scroling
      GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, true);
      GBC_Graphics_set_line_compare_interrupt_callback(graphics, handle_battle_scroll_interrupt);
      GBC_Graphics_stat_set_line_y_compare(graphics, 0);
      s_battle_enemy_scroll_x = 144;
      s_battle_player_scroll_x = 112;
      break;
    case PB_SLIDE:
      if (s_battle_frame < (144 / BATTLE_SCROLL_SPEED)) {
        s_battle_enemy_scroll_x -= BATTLE_SCROLL_SPEED;
        s_battle_player_scroll_x += BATTLE_SCROLL_SPEED;
        s_battle_frame++;
        for (uint8_t i = 0; i < 21; i++) {
          GBC_Graphics_oam_move_sprite(graphics, i, -BATTLE_SCROLL_SPEED, 0);
        }
      } else {
        GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
        for (uint8_t y = 0; y < 3; y++) {
          for (uint8_t x = 0; x < 7; x++) {
            GBC_Graphics_bg_move_tile(graphics, 0+x, 5+y, 0+x, 20+y, true);
          }
        }
        load_palette(graphics, s_player_pokemon, 1);
        load_palette(graphics, s_enemy_pokemon, 2);
        for (uint8_t i = 0; i < 21; i++) {
          GBC_Graphics_oam_hide_sprite(graphics, i);
        }
        s_battle_frame = 0;
        s_battle_state = PB_APPEAR;
      }
      break;
    case PB_APPEAR: {
      draw_battle_frames(graphics);
      draw_enemy_hp_bar(graphics, s_enemy_max_pokemon_health, s_enemy_pokemon_health);
      draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
      draw_exp_bar(graphics, 64, s_player_pokemon_exp);
      char level_text[8];
      if (s_enemy_pokemon_level < 100) {
        snprintf(level_text, 8, "|%-2d%c", s_enemy_pokemon_level, rand()%2 ? '^' : '+');
      } else {
        snprintf(level_text, 8, "%d%c", s_enemy_pokemon_level, rand()%2 ? '^' : '+');
      }
    #if defined(PBL_ROUND)
      draw_uint_string_at_location(graphics, GPoint(1, 1), s_enemy_pokemon_name, 11);
      draw_text_at_location(graphics, GPoint(4, 0), level_text);
    #else
      draw_uint_string_at_location(graphics, GPoint(1, 0), s_enemy_pokemon_name, 11);
      draw_text_at_location(graphics, GPoint(4, 1), level_text);
    #endif
      draw_uint_string_at_location(graphics, GPoint(8, 7), s_player_pokemon_name, 11);
      if (s_player_pokemon_level < 100) {
        snprintf(level_text, 8, "|%-2d%c", s_player_pokemon_level, rand()%2 ? '^' : '+');
      } else {
        snprintf(level_text, 8, "%d%c", s_player_pokemon_level, rand()%2 ? '^' : '+');
      }
      draw_text_at_location(graphics, GPoint(12, 8), level_text);
      char appear_dialogue[40];
      snprintf(appear_dialogue, 40, "Wild %s\nappeared!", s_enemy_pokemon_name);
      begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, appear_dialogue, true);
      s_prev_game_state = PG_BATTLE;
      s_game_state = PG_DIALOGUE;
      s_battle_state = PB_GO_POKEMON;
    } break;
    case PB_GO_POKEMON: {
      char go_dialogue[40];
      snprintf(go_dialogue, 40, "Do your best,\n%s!", s_player_pokemon_name);
      begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, go_dialogue, true);
      s_prev_game_state = PG_BATTLE;
      s_game_state = PG_DIALOGUE;
      s_battle_state = PB_PLAYER_TURN_PROMPT;
    } break;
    case PB_PLAYER_TURN_PROMPT:
      set_cursor_pos(0);
      draw_menu_rectangle(graphics, DIALOGUE_BOUNDS);
      draw_menu(graphics, BATTLE_MENU_BOUNDS, "FIGHT\n\nRUN", false, false);
      set_num_menu_items(2);
      s_battle_state = PB_PLAYER_TURN;
      break;
    case PB_PLAYER_TURN:
      break;
    case PB_PLAYER_MOVE: {
      char move_dialogue[40];
      snprintf(move_dialogue, 40, "%s\nattacked!", s_player_pokemon_name);
      s_player_pokemon_damage = calculate_damage(s_player_pokemon_level, s_player_pokemon_attack, s_enemy_pokemon_defense);
      s_clear_dialogue = false;
      begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, move_dialogue, false);
      s_prev_game_state = PG_BATTLE;
      s_game_state = PG_DIALOGUE;
      s_battle_state = PB_PLAYER_EFFECT;
      s_battle_frame = 0;
    } break;
    case PB_PLAYER_EFFECT:
      if (s_battle_frame <= 12) {
        if (s_battle_frame == 0) {
        #if defined(PBL_COLOR)
          GBC_Graphics_set_bg_palette(graphics, 7, 0xff, 0xff, 0xff, 0xff);
        #else
          GBC_Graphics_set_bg_palette(graphics, 7, 1, 1, 1, 1);
        #endif
        }
        if (s_battle_frame % 4 == 2) {
          render_pokemon(graphics, ENEMY_LOCATION, TILE_ENEMY_OFFSET, 7);
        } else if (s_battle_frame % 4 == 0) {
          render_pokemon(graphics, ENEMY_LOCATION, TILE_ENEMY_OFFSET, 2);
        }
        s_battle_frame++;
      } else {
        if (s_player_pokemon_damage > 0) {
          s_enemy_pokemon_health -= 1;
          s_player_pokemon_damage -= 1;
          if (s_enemy_pokemon_health == 0) {
            s_battle_state = PB_PLAYER_WIN;
            s_battle_frame = 0;
          }
          draw_enemy_hp_bar(graphics, s_enemy_max_pokemon_health, s_enemy_pokemon_health);
        } else {
          s_battle_state = PB_ENEMY_MOVE;
        }
      }
      break;
    case PB_ENEMY_MOVE: {
      char move_dialogue[40];
      snprintf(move_dialogue, 40, "%s\nattacked!", s_enemy_pokemon_name);
      s_enemy_pokemon_damage = calculate_damage(s_enemy_pokemon_level, s_enemy_pokemon_attack, s_player_pokemon_defense);
      s_clear_dialogue = false;
      begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, move_dialogue, false);
      s_prev_game_state = PG_BATTLE;
      s_game_state = PG_DIALOGUE;
      s_battle_state = PB_ENEMY_EFFECT;
      s_battle_frame = 0;
    } break;
    case PB_ENEMY_EFFECT:
      if (s_battle_frame <= 12) {
        if (s_battle_frame == 0) {
        #if defined(PBL_COLOR)
          GBC_Graphics_set_bg_palette(graphics, 7, 0xff, 0xff, 0xff, 0xff);
        #else
          GBC_Graphics_set_bg_palette(graphics, 7, 1, 1, 1, 1);
        #endif
        }
        if (s_battle_frame % 4 == 2) {
          render_pokemon(graphics, PLAYER_LOCATION, TILE_PLAYER_OFFSET, 7);
        } else if (s_battle_frame % 4 == 0) {
          render_pokemon(graphics, PLAYER_LOCATION, TILE_PLAYER_OFFSET, 1);
        }
        s_battle_frame++;
      } else {
        if (s_enemy_pokemon_damage > 0) {
          s_player_pokemon_health -= 1;
          s_enemy_pokemon_damage -= 1;
          if (s_player_pokemon_health == 0) {
            s_battle_state = PB_ENEMY_WIN;
            s_battle_frame = 0;
          }
          draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
        } else {
          s_battle_state = PB_PLAYER_TURN_PROMPT;
          s_battle_frame = 0;
        }
      }
      break;
    case PB_PLAYER_WIN: {
      if (s_battle_frame < 10) {
        for (short y = 5; y >= 0; y--) {
          for (uint8_t x = 0; x < 7; x++) {
            GBC_Graphics_bg_move_tile(graphics, 11+x, 0+y, 11+x, 0+y+1, false);
          }
        }
        for (uint8_t x = 0; x < 7; x++) {
          GBC_Graphics_bg_set_tile(graphics, 11+x, 0, 0);
        }
        s_battle_frame++;
      } else {
        for (uint8_t y = 0; y < 5; y++) {
          for (uint8_t x = 0; x < 11; x++) {
            GBC_Graphics_bg_set_tile(graphics, x, y, 0);
          }
        }
        char win_dialogue[40];
        snprintf(win_dialogue, 40, "Enemy %s\nfainted!", s_enemy_pokemon_name);
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, win_dialogue, true);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_battle_state = PB_FADEOUT;
        s_battle_frame = 0;
        s_stats_wins += 1;
        s_stats_battles += 1;
      }
    } break;
    case PB_ENEMY_WIN: {
      if (s_battle_frame < 10) {
        for (short y = 5; y >= 0; y--) {
          for (uint8_t x = 0; x < 7; x++) {
            GBC_Graphics_bg_move_tile(graphics, 0+x, 5+y, 0+x, 5+y+1, false);
          }
        }
        for (uint8_t x = 0; x < 7; x++) {
          GBC_Graphics_bg_set_tile(graphics, 0+x, 5, 0);
        }
        s_battle_frame++;
      } else {
        for (uint8_t y = 7; y < 12; y++) {
          for (uint8_t x = 7; x < 18; x++) {
            GBC_Graphics_bg_set_tile(graphics, x, y, 0);
          }
        }
        char lose_dialogue[40];
        snprintf(lose_dialogue, 40, "%s\nfainted!", s_player_pokemon_name);
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, lose_dialogue, true);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_battle_state = PB_FADEOUT;
        s_battle_frame = 0;
        s_stats_losses += 1;
        s_stats_battles += 1;
      }
    } break;
    case PB_RUN:
      if (rand() % 64 != 0) {
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "Got away safely!", true);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_battle_state = PB_FADEOUT;
        s_battle_frame = 0;
        s_stats_runs += 1;
        s_stats_battles += 1;
      } else {
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "Can't escape!", true);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_battle_state = PB_PLAYER_TURN_PROMPT;
      }
      break;
    case PB_FADEOUT:
      if (s_battle_frame < 5) {
        if (s_battle_frame == 0) {
          GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
        }
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, s_battle_frame);
      #else
        set_bg_palettes_to_color(graphics, 1);      
      #endif
        s_battle_frame++;
      } else {
        load_overworld(graphics);
        s_game_state = PG_PLAY;
      }
      break;
  }
}

static void save() {
  PokemonSaveData data = (PokemonSaveData) {
    .route_num = s_route_num,
    .player_x = s_player_x,
    .player_y = s_player_y,
    .player_direction = s_player_direction,
    .player_sprite_choice = s_player_sprite_choice,
    .player_palette_choice = s_player_palette_choice,
    .last_save = time(NULL),
    .battles = s_stats_battles,
    .wins = s_stats_wins,
    .losses = s_stats_losses,
    .runs = s_stats_runs,
  };
  persist_write_data(SAVE_KEY, &data, sizeof(PokemonSaveData));
  s_save_file_exists = true;
}

void Pokemon_step(GBC_Graphics *graphics) {
  s_step_frame = (s_step_frame + 1) % FRAME_SKIP;
  if (s_step_frame != 0) {
      return;
  }
  // s_poll_frame = (s_poll_frame + 1) % 8;
  // AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  // PlayerDirection old_direction = s_player_direction;
  if (s_player_mode == P_STAND && s_game_state == PG_PAUSE_QUEUED) {
    set_cursor_pos(0);
    draw_pause_menu(graphics);
    s_game_state = PG_PAUSE;
    s_menu_state = PM_BASE;
  }
  switch (s_game_state) {
    case PG_PAUSE_QUEUED:
    case PG_PLAY:
      play(graphics);
      break;
    case PG_PAUSE:
      break;
    case PG_DIALOGUE:
      if (get_dialogue_state() != D_IDLE) {
        step_dialogue(graphics, s_select_pressed);
      } else {
        if (s_prev_game_state == PG_PLAY) {
          load_screen(graphics);
          s_game_state = PG_PLAY;
          s_select_pressed = false;
        } else if (s_prev_game_state == PG_PAUSE) {
          switch(s_menu_state) {
            case PM_SAVE_CONFIRM:
              set_cursor_pos(0);
              draw_menu(graphics, SAVE_MENU_BOUNDS, "YES\n\nNO", false, true);
              set_num_menu_items(2);
              s_game_state = PG_PAUSE;
              s_select_pressed = false;
              break;
            case PM_SAVE_OVERWRITE:
              set_cursor_pos(0);
              draw_menu(graphics, SAVE_MENU_BOUNDS, "YES\n\nNO", false, true);
              set_num_menu_items(2);
              s_game_state = PG_PAUSE;
              s_select_pressed = false;
              break;
            case PM_SAVING:
              save();
              begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "Game saved\nsuccessfully.", true);
              s_prev_game_state = PG_PAUSE;
              s_game_state = PG_DIALOGUE;
              s_menu_state = PM_BASE;
              s_select_pressed = false;
              break;
            default:
              draw_pause_menu(graphics);
              s_game_state = PG_PAUSE;
              s_select_pressed = false;
              break;
          }
        } else if (s_prev_game_state == PG_BATTLE) {
          if (s_clear_dialogue) {
            draw_menu_rectangle(graphics, DIALOGUE_BOUNDS);
          }
          s_clear_dialogue = true;
          s_game_state = PG_BATTLE;
          s_select_pressed = false;
        }
      }
      break;
    case PG_BATTLE:
      battle(graphics);
      break;
    default:
      break;
  }
}

void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed) {
  s_select_pressed = pressed;
}

static void draw_pebble_info(GBC_Graphics *graphics) {
  char text_buffer[20] = {0};
  // load_screen(graphics);
  draw_menu_rectangle(graphics, GRect(PEBBLE_ROOT_X, PEBBLE_ROOT_Y, 16, 11));
  
  char time_buffer[9] = {0};
  clock_copy_time_string(time_buffer, 9);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 2), time_buffer);

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(text_buffer, sizeof(text_buffer), "%A", tick_time);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 4), text_buffer);
  strftime(text_buffer, sizeof(text_buffer), "%b %d, %Y", tick_time);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 6), text_buffer);

  snprintf(text_buffer, sizeof(text_buffer), "Battery: %d%%", battery_state_service_peek().charge_percent);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 8), text_buffer);
}

static void draw_stats(GBC_Graphics *graphics) {
  char text_buffer[20] = {0};
  draw_menu_rectangle(graphics, GRect(PEBBLE_ROOT_X, PEBBLE_ROOT_Y, 16, 11));

  snprintf(text_buffer, sizeof(text_buffer), "Battles:%4d", s_stats_battles);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 2), text_buffer);

  snprintf(text_buffer, sizeof(text_buffer), "Wins:%7d", s_stats_wins);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 4), text_buffer);

  snprintf(text_buffer, sizeof(text_buffer), "Losses:%5d", s_stats_losses);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 6), text_buffer);

  snprintf(text_buffer, sizeof(text_buffer), "Runs:%7d", s_stats_runs);
  draw_text_at_location(graphics, GPoint(PEBBLE_ROOT_X + 2, PEBBLE_ROOT_Y + 8), text_buffer);
}

void Pokemon_handle_select_click(GBC_Graphics *graphics) {
  switch(s_game_state) {
    case PG_PAUSE:
      switch(s_menu_state) {
        case PM_BASE:
          switch (get_cursor_pos()) {
            case 0: // Resume
              s_game_state = PG_PLAY;
              load_screen(graphics);
              s_select_pressed = false;
              break;
            case 1: // Pebble
              draw_pebble_info(graphics);
              s_menu_state = PM_PEBBLE;
              break;
            case 2: // Stats
              draw_stats(graphics);
              s_menu_state = PM_STATS;
              break;
            case 3: // Save
              begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "Do you want to\nsave the game?", false);
              s_prev_game_state = s_game_state;
              s_menu_state = PM_SAVE_CONFIRM;
              s_game_state = PG_DIALOGUE;
              break;
            case 4: // Quit
              window_stack_pop(true);
              break;
          }
          break;
        case PM_PEBBLE:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          break;
        case PM_STATS:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          break;
        case PM_SAVE_CONFIRM:
          switch (get_cursor_pos()) {
            case 0: // Yes
              s_select_pressed = false;
              set_cursor_pos(3);
              draw_pause_menu(graphics);
              PokemonSaveData _data;
              if (load(&_data)) {
                s_menu_state = PM_SAVE_OVERWRITE;
                begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "There is already\na save file, is\nit OK to over-\nwrite?", false);
              } else {
                s_menu_state = PM_SAVING;
                begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "SAVING, DO NOT\nCLOSE THE APP...", false);
              }
              s_prev_game_state = s_game_state;
              s_game_state = PG_DIALOGUE;
              break;
            case 1: // No
              s_select_pressed = false;
              set_cursor_pos(3);
              draw_pause_menu(graphics);
              s_menu_state = PM_BASE;
              break;
          }
          break;
        case PM_SAVE_OVERWRITE:
          switch (get_cursor_pos()) {
            case 0: // Yes
              s_select_pressed = false;
              set_cursor_pos(3);
              draw_pause_menu(graphics);
              s_menu_state = PM_SAVING;
              begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "SAVING, DO NOT\nCLOSE THE APP...", false);
              s_prev_game_state = s_game_state;
              s_game_state = PG_DIALOGUE;
              break;
            case 1: // No
              s_select_pressed = false;
              set_cursor_pos(3);
              draw_pause_menu(graphics);
              s_menu_state = PM_BASE;
              break;
          }
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      if (s_save_file_exists) {
        switch(get_cursor_pos()) {
          case 0: {// Continue
            PokemonSaveData data;
            load(&data);
            s_route_num = data.route_num;
            s_player_x = data.player_x;
            s_player_y = data.player_y;
            s_player_direction = data.player_direction;
            s_player_sprite_choice = data.player_sprite_choice;
            s_player_palette_choice = data.player_palette_choice;
            s_stats_battles = data.battles;
            s_stats_wins = data.wins;
            s_stats_losses = data.losses;
            s_stats_runs = data.runs;
            load_game(graphics);
            s_game_state = PG_PLAY;
            s_select_pressed = false;
          } break;
          case 1: // New
            s_player_x = PLAYER_ORIGIN_X;
            s_player_y = PLAYER_ORIGIN_Y;
            new_player_sprites(graphics);
            load_game(graphics);
            s_game_state = PG_PLAY;
            s_select_pressed = false;
            break;
          case 2: // Quit
            window_stack_pop(true);
            break;
        }
      } else {
        switch(get_cursor_pos()) {
          case 0: // New
            s_player_x = PLAYER_ORIGIN_X;
            s_player_y = PLAYER_ORIGIN_Y;
            new_player_sprites(graphics);
            load_game(graphics);
            s_game_state = PG_PLAY;
            s_select_pressed = false;
            break;
          case 1: // Quit
            window_stack_pop(true);
            break;
        }
      }
    case PG_DIALOGUE:
      handle_input_dialogue(graphics);
      break;
    case PG_BATTLE:
      switch(s_battle_state) {
        case PB_PLAYER_TURN:
          switch (get_cursor_pos()) {
            case 0: // Fight
              s_select_pressed = false;
              s_battle_state = PB_PLAYER_MOVE;
              break;
            case 1: // Run
              s_select_pressed = false;
              s_battle_state = PB_RUN;
              break;
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void Pokemon_handle_down(GBC_Graphics *graphics) {
  switch (s_game_state) {
    case PG_PLAY:
      if (s_player_mode == P_STAND) {
        s_player_direction = (s_player_direction - 1) & 3;
        set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
      }
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_down(graphics);
          draw_menu_info(graphics);
          break;
        case PM_SAVE_CONFIRM:
        case PM_SAVE_OVERWRITE:
          move_cursor_down(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      move_cursor_down(graphics);
      break;
    case PG_BATTLE:
      switch (s_battle_state) {
        case PB_PLAYER_TURN:
          move_cursor_down(graphics);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void Pokemon_handle_up(GBC_Graphics *graphics) {
  switch (s_game_state) {
    case PG_PLAY:
      if (s_player_mode == P_STAND) {
        s_player_direction = (s_player_direction + 1) & 3;
        set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
      }
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_up(graphics);
          draw_menu_info(graphics);
          break;
        case PM_SAVE_CONFIRM:
        case PM_SAVE_OVERWRITE:
          move_cursor_up(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      move_cursor_up(graphics);
      break;
    case PG_BATTLE:
      switch (s_battle_state) {
        case PB_PLAYER_TURN:
          move_cursor_up(graphics);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void Pokemon_handle_tap(GBC_Graphics *graphics) {
  if (s_game_state == PG_PLAY) {
    new_player_sprites(graphics);
  }
}

void Pokemon_handle_focus_lost(GBC_Graphics *graphics) {
  s_select_pressed = false;
}

void Pokemon_handle_back(GBC_Graphics *graphics) {
  if (get_dialogue_state() != D_IDLE) {
    handle_input_dialogue(graphics);
    return;
  }
  switch (s_game_state) {
    case PG_PLAY:
      s_game_state = PG_PAUSE_QUEUED;
      break;
    case PG_PAUSE:
      switch(s_menu_state) {
        case PM_BASE:
          s_game_state = PG_PLAY;
          load_screen(graphics);
          s_select_pressed = false;
          break;
        case PM_PEBBLE:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          break;
        case PM_STATS:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      window_stack_pop(true);
      break;
    default:
      break;
  }
}