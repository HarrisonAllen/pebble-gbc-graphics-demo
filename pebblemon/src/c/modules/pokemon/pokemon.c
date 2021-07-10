#include "pokemon.h"
#include "menu.h"
#include "world.h"
#include "sprites.h"
#include "objects.h"
#include "sprite_decompressor/decompressor.h"
#include "enums.h"
#include "animations.h"
#include "items.h"

static PlayerDirection s_player_direction = D_DOWN;
static PlayerMode s_player_mode;
static uint8_t s_walk_frame, s_poll_frame, s_battle_frame, s_anim_frame;
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
static PokemonTreeState s_tree_state;
static bool s_save_file_exists;
static uint16_t s_player_pokemon, s_enemy_pokemon;
static uint8_t s_player_pokemon_name[11], s_enemy_pokemon_name[11];
static uint8_t s_step_frame;
static uint8_t s_battle_enemy_scroll_x, s_battle_player_scroll_x;
static uint8_t s_enemy_pokemon_level;
static uint16_t s_player_max_pokemon_health, s_enemy_max_pokemon_health;
static uint16_t s_player_pokemon_health, s_enemy_pokemon_health;
static uint8_t s_player_pokemon_damage, s_enemy_pokemon_damage;
static uint16_t s_player_pokemon_attack, s_enemy_pokemon_attack;
static uint16_t s_player_pokemon_defense, s_enemy_pokemon_defense;
static bool s_clear_dialogue = true;
static uint8_t s_cur_bg_palettes[PALETTE_BANK_SIZE];
static uint16_t s_stats_battles, s_stats_wins, s_stats_losses, s_stats_runs;
static uint8_t s_route_num = 3;
static uint8_t s_warp_route;
static uint16_t s_warp_x, s_warp_y;
static uint8_t s_player_level = 1;
static int s_player_exp = 0;
static bool s_move_mode_toggle = true, s_move_toggle;
static bool s_turn_mode_tilt;
static bool s_backlight_on;
static uint8_t s_player_sprite = 0;
static uint8_t s_text_speed = 1;
static int s_to_next_level, s_to_cur_level;
static int s_exp_gained;
static bool s_up_press_queued, s_down_press_queued;
static uint8_t s_escape_odds;
static uint8_t s_player_items; // = SET_ITEM(SET_ITEM(0, ITEM_ID_RUNNING_SHOES), ITEM_ID_LUCKY_EGG);
static bool s_player_goes_first;
int s_accel_x_cal, s_accel_y_cal;
static uint8_t s_tree_frame;
static uint16_t s_health_to_gain;
static bool s_eaten_berry;


// TODO: 
// - Add in a check for if a tile is out of bounds, then clamp it to bounds instead of error->crash
// - Add in a window overlay when entering a new route
// - Level 100 victory

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
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
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

static void draw_black_rectangle(GBC_Graphics *graphics, GRect rect_bounds, bool overlay) {
  uint8_t attrs = GBC_Graphics_attr_make(7, 1, 0, 0, overlay ? 1 : 0);
  uint8_t root_x = rect_bounds.origin.x;
  uint8_t root_y = rect_bounds.origin.y;

  for (uint8_t y = 0; y < rect_bounds.size.h; y++) {
    for (uint8_t x = 0; x < rect_bounds.size.w; x++) {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + x, root_y + y, 0, attrs);
    }
  }
}

static void load_screen(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
  }
#if defined(PBL_COLOR)
  GBC_Graphics_set_sprite_palette_array(graphics, 4, &palettes[s_route_num][2*PALETTE_SIZE]);
#endif
  GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
  draw_black_rectangle(graphics, GRect(0, 0, 32, 32), false);
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
  s_player_palette = pokemon_trainer_palettes[s_player_sprite];
  GBC_Graphics_set_sprite_palette(graphics, 0, s_player_palette[0], s_player_palette[1], s_player_palette[2], s_player_palette[3]);
}

static void load_player_sprites(GBC_Graphics *graphics) {
  reset_player_palette(graphics);

  uint16_t spritesheet_offset = pokemon_trainer_sprites[s_player_sprite] * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET,
    spritesheet_offset, POKEMON_TILES_PER_SPRITE * POKEMON_SPRITES_PER_TRAINER, 10, TILE_BANK_SPRITES); // offset 10 for effects and items
}

static void new_player_sprites(GBC_Graphics *graphics) {
  s_player_sprite = 0; //rand() % 22;
  // s_player_color = rand() % 6;
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
    GBC_Graphics_oam_set_sprite_tile(graphics, 2 + i, (new_tile * POKEMON_TILES_PER_SPRITE) + i + 10);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, 2 + i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 2, 3);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 4, 5);
  }
}

static void hide_preview_sprites(GBC_Graphics *graphics, uint8_t oam_offset) {
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_hide_sprite(graphics, oam_offset + i);
  }
}

static void set_preview_sprites(GBC_Graphics *graphics, GPoint pos, PlayerDirection dir, bool walk_sprite, bool x_flip, uint8_t oam_offset) {
  uint8_t new_tile = pokemon_trainer_sprite_offsets[dir + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, oam_offset + i, (new_tile * POKEMON_TILES_PER_SPRITE) + i + 10);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, oam_offset + i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, oam_offset, oam_offset + 1);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, oam_offset + 2, oam_offset + 3);
  }

  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      GBC_Graphics_oam_set_sprite_pos(graphics, oam_offset + x + 2 * y, (pos.x + x) * 8 + 8, (pos.y + y) * 8 + 16);
      GBC_Graphics_oam_set_sprite_attrs(graphics, oam_offset + x + 2 * y, GBC_Graphics_attr_make(0, 3, false, false, false));
      GBC_Graphics_bg_set_tile_priority(graphics, pos.x + x, pos.y + y, false);
    }
  }
}

static void hide_item(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_hide_sprite(graphics, 20+i);
  }
}

static void render_items(GBC_Graphics *graphics) {
  for (uint8_t i = 0; i < 8; i++) {
    const int16_t *item = items[i];
    int item_x = item[1] << 4;
    int item_y = item[2] << 4;
    if (s_route_num == item[0] && !HAS_ITEM(s_player_items, item[3])) {
      if (!item[4]) {
        int x_diff = item_x - s_player_x;
        int y_diff = item_y - s_player_y;
        if (abs(x_diff) <= 80 && abs(y_diff) <= 80) {
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 0, x_diff + 72, y_diff + 76);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 1, x_diff + 72 + 8, y_diff + 76);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 2, x_diff + 72, y_diff + 76 + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 20 + 3, x_diff + 72 + 8, y_diff + 76 + 8);
        }
      }
    }
  }
}

static bool load(PokemonSaveData *data) {
  return persist_read_data(SAVE_KEY, data, sizeof(PokemonSaveData)) != E_DOES_NOT_EXIST;
}

static uint8_t get_block_type(uint8_t *map, uint16_t x, uint16_t y) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return block_types[s_route_num][block];
}

static void set_block(uint8_t *map, uint16_t x, uint16_t y, uint8_t replacement) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Chunk %d and block %d", chunk, chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2);
  chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2] = replacement;
}

static void load_overworld(GBC_Graphics *graphics) {
  load_screen(graphics);
  
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  load_player_sprites(graphics);

  // Create grass and shadow effect sprites
  uint16_t spritesheet_offset = POKEMON_SPRITELET_GRASS * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset, 1, 0, TILE_BANK_SPRITES);
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset+2, 1, 1, TILE_BANK_SPRITES);
  GBC_Graphics_oam_set_sprite(graphics, 0, 0, 0, 0, GBC_Graphics_attr_make(6, 3, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 1, 0, 0, 0, GBC_Graphics_attr_make(6, 3, true, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 6, 0, 0, 1, GBC_Graphics_attr_make(0, 3, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 7, 0, 0, 1, GBC_Graphics_attr_make(0, 3, true, false, false));
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 6, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
  #else
    GBC_Graphics_set_sprite_palette(graphics, 6, 1, 1, 0, 0);
  #endif

  // Create item sprites
  spritesheet_offset = ITEM_SPRITE_POKEBALL * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET, spritesheet_offset, 8, 2, TILE_BANK_SPRITES);
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite(graphics, 20+i, 0, 0, 2+i, GBC_Graphics_attr_make(5, 3, false, false, false));
  }
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite(graphics, 24+i, 0, 0, 6+i, GBC_Graphics_attr_make(4, 3, false, false, false));
  }
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 5, 0b11111111, 0b11111001, 0b11110000, 0b11000000);
    // To make sure the palete is correct for forest vs route, so it's set to background palette in load_screen instead
  #else
    GBC_Graphics_set_sprite_palette(graphics, 5, 1, 1, 0, 0);
    GBC_Graphics_set_sprite_palette(graphics, 4, 1, 1, 0, 0);
  #endif
  render_items(graphics);

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

static int check_for_object(uint16_t target_x, uint16_t target_y) {
  uint16_t block_x = target_x >> 4;
  uint16_t block_y = target_y >> 4;
  for (uint16_t i = 0; i < sizeof(objects) >> 2; i++) {
    if (s_route_num == objects[i][0] && block_x == objects[i][1] && block_y == objects[i][2]) {
      return i;
    }
  }
  return -1;
}

static void load_resources(GBC_Graphics *graphics) {
  ResHandle handle = resource_get_handle(map_files[s_route_num]);
  size_t res_size = resource_size(handle);
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
  s_world_map = (uint8_t*)malloc(res_size);
  resource_load(handle, s_world_map, res_size);
    
  int object_num = check_for_object(s_player_x, s_player_y);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Found object %d at (%d, %d) and is %d", object_num, object_num != -1 ? objects[object_num][3] : -1);
  if (object_num != -1 && objects[object_num][3] == PO_TREE) { // Make sure player doesn't spawn in tree
    set_block(s_world_map, s_player_x, s_player_y, replacement_blocks[s_route_num]);
  }

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

#if defined(PBL_COLOR)
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
#endif

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

  ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_MENU_TILESHEET);
  size_t res_size = resource_size(handle);
  uint16_t tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_MENU_TILESHEET, 0, tiles_to_load, 0, TILE_BANK_MENU);

  init_anim_tiles(graphics, TILE_BANK_ANIMS, TILE_OFFSET_ANIMS);

  handle = resource_get_handle(RESOURCE_ID_DATA_WORLD_TILESHEET);
  res_size = resource_size(handle);
  tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_WORLD_TILESHEET, 0, tiles_to_load, 0, TILE_BANK_WORLD);

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

    draw_textbox(graphics, GRect(START_INFO_ROOT_X, START_INFO_ROOT_Y, 16, 6), GPoint(1, 1), "Last save:", false);
    struct tm *tick_time = localtime(&data.last_save);
    char text_buffer[20] = {0};
    strftime(text_buffer, sizeof(text_buffer), "%a %b %d", tick_time);
    draw_text_at_location(graphics, GPoint(START_INFO_ROOT_X + 3, START_INFO_ROOT_Y + 2), text_buffer);
    strftime(text_buffer, sizeof(text_buffer), "%r", tick_time);
    draw_text_at_location(graphics, GPoint(START_INFO_ROOT_X + 3, START_INFO_ROOT_Y + 3), text_buffer);
    snprintf(text_buffer, sizeof(text_buffer), "Level: %d", data.player_level);
    draw_text_at_location(graphics, GPoint(START_INFO_ROOT_X + 1, START_INFO_ROOT_Y + 4), text_buffer);
  } else {    
    draw_menu(graphics, GRect(START_MENU_ROOT_X, START_MENU_ROOT_Y, 12, 6), "NEW GAME\n\nQUIT", false, false);
    set_num_menu_items(2);
  }

  // load_game(graphics);
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

static int check_for_item(uint16_t target_x, uint16_t target_y) {
  uint16_t block_x = target_x >> 4;
  uint16_t block_y = target_y >> 4;
  for (uint16_t i = 0; i < sizeof(items) >> 2; i++) {
    if (s_route_num == items[i][0] && block_x == items[i][1] && block_y == items[i][2]) {
      return i;
    }
  }
  return -1;
}

#if defined(PBL_COLOR)
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

#else

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
#endif

static void play(GBC_Graphics *graphics) {
  s_anim_frame = (s_anim_frame + 1) % 8;
  if (s_anim_frame == 0) {
    animate_tiles(graphics, TILE_BANK_WORLD, s_route_num);
  }
  s_poll_frame = (s_poll_frame + 1) % 8;
  if (s_player_mode == P_STAND) {
    if (s_up_press_queued) {
      s_player_direction = (s_player_direction + 1) & 3;
      set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
      s_up_press_queued = false;
    }
    if (s_down_press_queued) {
      s_player_direction = (s_player_direction - 1) & 3;
      set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
      s_down_press_queued = false;
    }
    if (s_select_pressed || (s_move_mode_toggle && s_move_toggle)) {
      if (s_turn_mode_tilt) {
        AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
        PlayerDirection old_direction = s_player_direction;
        accel_service_peek(&accel);
        int cal_accel_x = accel.x - s_accel_x_cal;
        int cal_accel_y = accel.y - s_accel_y_cal;
        if (abs(cal_accel_x) > abs(cal_accel_y)) {
          if (cal_accel_x < -150) {
            s_player_direction = D_LEFT;
          } else if (cal_accel_x > 150) {
            s_player_direction = D_RIGHT;
          }
        } else {
          if (cal_accel_y < -150) {
            s_player_direction = D_DOWN;
          } else if (cal_accel_y > 150) {
            s_player_direction = D_UP;
          }
        }
        if (old_direction != s_player_direction) {
          set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
        }
      }
      s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
      s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
      
      s_player_mode = HAS_ITEM(s_player_items, ITEM_ID_RUNNING_SHOES) ? P_RUN : P_WALK;
      s_walk_frame = 0;
      if (s_player_sprite != 21) {
        s_flip_walk = !s_flip_walk;
      }
      GBC_Graphics_oam_set_sprite_priority(graphics, 4, false);
      GBC_Graphics_oam_set_sprite_priority(graphics, 5, false);

      int item_num = check_for_item(s_target_x+8, s_target_y);
      if ((item_num != -1) && !HAS_ITEM(s_player_items, item_num)) {
        s_player_items = SET_ITEM(s_player_items, item_num);
        hide_item(graphics);
        if (s_move_mode_toggle) {
          s_move_toggle = false;
        }
        load_screen(graphics);
        begin_dialogue(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, item_num, true);
        s_prev_game_state = s_game_state;
        s_game_state = PG_DIALOGUE;
        s_player_mode = P_STAND;
        s_can_move = false;
      } else {
        PokemonSquareInfo current_block_type = get_block_type(s_world_map, s_player_x, s_player_y);
        PokemonSquareInfo target_block_type = get_block_type(s_world_map, s_target_x+8, s_target_y);
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "Block at %d, %d yielded type %d", s_target_x+8, s_target_y, target_block_type);
        if (target_block_type == OBJECT) {
          int object_num = check_for_object(s_target_x+8, s_target_y);
          // APP_LOG(APP_LOG_LEVEL_DEBUG, "Check at %d, %d yielded object %d", s_target_x+8, s_target_y, object_num);
          if (object_num != -1) {
            if (s_move_mode_toggle) {
              s_move_toggle = false;
            }
            PokemonObjectTypes object_type = objects[object_num][3];
            const int16_t *data = &objects[object_num][4];
            switch (object_type) {
              case PO_NONE:
                break;
              case PO_TREE:
                load_screen(graphics);
                s_player_mode = P_STAND;
                s_can_move = false;
                if (HAS_ITEM(s_player_items, ITEM_ID_CUT)) {
                  s_prev_game_state = PG_TREE;
                  s_tree_state = PT_CONFIRM;
                  s_game_state = PG_DIALOGUE;
                  begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "This tree can be\nCUT!\nWould you like\nto use CUT?", false);
                } else {
                  s_prev_game_state = s_game_state;
                  s_game_state = PG_DIALOGUE;
                  begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "This tree can be\nCUT!", true);
                }
                break;
              case PO_TEXT:
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
                  load_blocks_in_direction(graphics, s_player_direction);
                } else {
                  s_player_mode = P_WARP;
                }
                s_can_move = true;
              } break;
              default:
                s_can_move = false;
                break;
            }
          } else {
            s_can_move = false;
          }
        } else {
          if ((target_block_type == CLIFF_S && s_player_direction == D_DOWN)
              || (target_block_type == CLIFF_W && s_player_direction == D_LEFT)
              || (target_block_type == CLIFF_E && s_player_direction == D_RIGHT)) {
            s_target_x += direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
            s_target_y += direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
            s_player_mode = P_JUMP;
            s_can_move = true;
          } else {
            s_can_move = (target_block_type == WALK || target_block_type == GRASS
                          || (target_block_type == CLIFF_N && s_player_direction != D_DOWN));
            s_can_move &= !(current_block_type == CLIFF_N && s_player_direction == D_UP);
          }
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
    }
  }
  
  switch(s_player_mode) {
    case P_STAND:
      if (s_turn_mode_tilt) {
        AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
        PlayerDirection old_direction = s_player_direction;
        if (s_poll_frame == 0) { // Reduce polling rate to reduce battery drain
          accel_service_peek(&accel);
          if (abs(accel.x) > abs(accel.y)) {
            if (accel.x < -150) {
              s_player_direction = D_LEFT;
            } else if (accel.x > 150) {
              s_player_direction = D_RIGHT;
            }
          } else {
            if (accel.y < -150) {
              s_player_direction = D_DOWN;
            } else if (accel.y > 150) {
              s_player_direction = D_UP;
            }
          }
          if (old_direction != s_player_direction) {
            set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
          }
        }
      }
      break;
    case P_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
        render_items(graphics);
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
            break;
          default:
            break;
        }
      }
      if (s_walk_frame == 7 && s_can_move && (s_route_num == 0 || s_route_num == 1 || get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS)) {
        if (ENCOUNTERS_ENABLED && rand() % WILD_ODDS == 0) {
          load_screen(graphics);
          s_game_state = PG_BATTLE;
          s_battle_state = PB_FLASH;
          s_battle_frame = 0;
          if (s_move_mode_toggle) {
            s_move_toggle = false;
          }
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
    case P_RUN:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 4;
        s_player_y += direction_to_point(s_player_direction).y * 4;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 4, direction_to_point(s_player_direction).y * 4);
        render_items(graphics);
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS) { // grass
        switch(s_walk_frame) {
          case 1:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
            break;
          case 2:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x - 1, s_player_sprite_y + 5);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 9, s_player_sprite_y + 5);
            break;
          case 3:
          #if defined(PBL_COLOR)
            GBC_Graphics_oam_hide_sprite(graphics, 0);
            GBC_Graphics_oam_hide_sprite(graphics, 1);
          #else
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
          #endif
            break;
          default:
            break;
        }
      }
      if (s_walk_frame == 3 && s_can_move && (s_route_num == 0 || s_route_num == 1 || get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS)) {
        if (ENCOUNTERS_ENABLED && rand() % WILD_ODDS == 0) {
          load_screen(graphics);
          s_game_state = PG_BATTLE;
          s_battle_state = PB_FLASH;
          s_battle_frame = 0;
          if (s_move_mode_toggle) {
            s_move_toggle = false;
          }
        }
      }
      switch(s_walk_frame) {
        case 3:
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
        case 0:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 1:
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
        render_items(graphics);
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS && s_walk_frame == 15) { // grass
        if (s_can_move && ENCOUNTERS_ENABLED && (rand() % WILD_ODDS == 0)) {
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
        render_items(graphics);
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
      if (s_walk_frame < 5) {
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
      } else if (s_walk_frame == 5) {  
        s_route_num = s_warp_route;
        s_player_x = s_warp_x;
        s_player_y = s_warp_y;
        s_target_x = s_warp_x;
        s_target_y = s_warp_y;
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "Warping to Route %d, Pos (%d, %d)", s_route_num, s_player_x, s_player_y);
        load_resources(graphics);
        load_blocks_in_direction(graphics, s_player_direction);
        GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
      #if defined(PBL_BW)
        set_bg_palettes_to_color(graphics, 1);
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
        }
      #else 
        lerp_bg_palettes_to_color(graphics, 0b11111111, 4);
        lerp_player_palette_to_color(graphics, 0b11111111, 4);
      #endif
      } else {
      #if defined(PBL_COLOR)
        lerp_bg_palettes_to_color(graphics, 0b11111111, 10 - s_walk_frame);
        lerp_player_palette_to_color(graphics, 0b11111111, 10 - s_walk_frame);
      #endif
      }
      s_walk_frame++;
      if (s_walk_frame == 11) {
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
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "View game\n\nstatistics", true);
      break;
    case 1:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "View time &\n\nPebble info", true);
      break;
    case 2:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Change\n\nsettings", true);
      break;
    case 3:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Contains\n\nitem", true);
      break;
    case 4:
      draw_textbox(graphics, GRect(INFO_ROOT_X, INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Save & quit", true);
      break;
    default:
      break;
  }
}

static void draw_pause_menu(GBC_Graphics *graphics) {
  load_screen(graphics);
  draw_menu(graphics, GRect(MENU_ROOT_X, MENU_ROOT_Y, 9, NUM_MENU_ITEMS * 2 + 2), "STATS\n\nPEBBLE\n\nOPTION\n\nPACK\n\nQUIT", false, false);
  set_num_menu_items(NUM_MENU_ITEMS);
  draw_menu_info(graphics);
}

void draw_option_menu(GBC_Graphics *graphics) {
  draw_menu(graphics, GRect(OPTION_ROOT_X, OPTION_ROOT_Y, 18, 14), "MOVE MODE\n\nTURN MODE\n\nTEXT SPEED\n\nBACKLIGHT\n\nSPRITE\n\nCANCEL", false, false);
  set_num_menu_items(6);
  draw_text_at_location(graphics, GPoint(OPTION_ROOT_X+9, OPTION_ROOT_Y+3), s_move_mode_toggle ? ":TOGGLE" : ":HOLD");
  draw_text_at_location(graphics, GPoint(OPTION_ROOT_X+9, OPTION_ROOT_Y+5), s_turn_mode_tilt ? ":TILT" : ":BUTTONS");
  draw_text_at_location(graphics, GPoint(OPTION_ROOT_X+9, OPTION_ROOT_Y+7), s_text_speed == 0 ? ":SLOW" : s_text_speed == 1 ? ":MID" : ":FAST");
  draw_text_at_location(graphics, GPoint(OPTION_ROOT_X+9, OPTION_ROOT_Y+9), s_backlight_on ? ":ON" : ":AUTO");
  char data[4] = {0};
  snprintf(data, 4, ":%d", s_player_sprite);
  draw_text_at_location(graphics, GPoint(OPTION_ROOT_X+9, OPTION_ROOT_Y+11), data);
  set_preview_sprites(graphics, GPoint(OPTION_ROOT_X+13, OPTION_ROOT_Y+10), D_DOWN, false, false, 8);
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

static uint16_t calculate_damage(uint8_t level, uint16_t attack, uint16_t defense) {
  uint16_t crit = (rand()%20==0) ? 1.5 : 1;
  return (uint16_t)(((((2.0 * level / 5.0 + 2.0) * attack * 80.0 / defense) / 50) + 2) * (rand()%16 + 85) / 100) * crit;
}

static uint16_t calculate_exp_gain(uint8_t enemy_level, bool lucky) {
  uint16_t effort_value = rand()%200 + 100;
  return effort_value * enemy_level * (lucky ? 2 : 1.0) / 6;
}

static int cube(int x) {
  return x*x*x;
}

static void generate_pokemon_stats() {
  s_to_next_level = cube(s_player_level+1);
  s_to_cur_level = s_player_level == 1 ? 0 : cube(s_player_level);
  int enemy_level;
  if (s_player_level >= 5) {
    enemy_level = s_player_level + rand()%9 - 4;
    if (enemy_level < 1) {
      enemy_level = 1;
    } else if (enemy_level > 100) {
      enemy_level = 100;
    }
  } else {
    enemy_level = s_player_level;
  }
  s_enemy_pokemon_level = enemy_level;
  
  s_player_max_pokemon_health = calculate_health(s_player_level);
  s_enemy_max_pokemon_health = calculate_health(s_enemy_pokemon_level);
  s_player_pokemon_health = s_player_max_pokemon_health;
  s_enemy_pokemon_health = s_enemy_max_pokemon_health;
  s_player_pokemon_attack = (rand()%15+5)*(s_player_level/10.0+1)*(HAS_ITEM(s_player_items, ITEM_ID_PROTEIN) ? 1.25 : 1.0);
  s_enemy_pokemon_attack = (rand()%15+5)*(s_player_level/10.0+1);
  s_player_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1)*(HAS_ITEM(s_player_items, ITEM_ID_IRON) ? 1.25 : 1.0);
  s_enemy_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1);
}

static void distortion_callback(GBC_Graphics *graphics) {
  uint8_t screen_pos = GBC_Graphics_stat_get_line_y_compare(graphics);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
  GBC_Graphics_bg_move(graphics, sin_lookup(screen_pos*2048) * (s_battle_frame * s_battle_frame) / TRIG_MAX_RATIO, 0);
  GBC_Graphics_stat_set_line_y_compare(graphics, screen_pos+1);
}

// Implementation of Bresenham's Line Drawing Algorithm, from wikipedia
static void draw_line(GBC_Graphics *graphics, int x0, int y0, int x1, int y1) {
  uint8_t attrs = GBC_Graphics_attr_make(7, 1, 0, 0, 0);
  int dx = abs(x1-x0);
  int sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0);
  int sy = y0<y1 ? 1 : -1;

  int err = dx+dy;
  while (true) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, x0, y0, 0, attrs);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2*err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

int clamp(int to_clamp, int min_val, int max_val) {
  if (to_clamp < min_val) {
    return min_val;
  }
  if (to_clamp > max_val) {
    return max_val;
  }
  return to_clamp;
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
        lerp_bg_palettes_to_color(graphics, 0b11000000, 3);
      } else if (frame_mod == 5 || frame_mod == 7) {
        lerp_bg_palettes_to_color(graphics, 0b11111111, 2);
      } else if (frame_mod == 6) {
        lerp_bg_palettes_to_color(graphics, 0b11111111, 3);
      }
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
    #endif
      s_battle_frame++;
      if (s_battle_frame == 24) {
        for (uint8_t i = 0; i < 8; i++) {
          GBC_Graphics_set_bg_palette_array(graphics, i, &palettes[s_route_num][i*PALETTE_SIZE]);
        }
        switch (s_route_num) {
          case 0: // Cave
            s_battle_state = PB_ANIM_BOX;
            break;
          case 1: // Forest
            s_battle_state = PB_ANIM_DISTORT;
            break;
          case 2: // National Park
            s_battle_state = PB_ANIM_RADIAL;
            break;
          default: // Routes
            s_battle_state = PB_ANIM_FADE;
            break;
        }
        s_battle_frame = 0;
      }
    } break;
    case PB_ANIM_BOX: // Box that expands outwards
      if (s_battle_frame <= 8) {
        draw_black_rectangle(graphics, GRect(8 - s_battle_frame, 8 - s_battle_frame, (s_battle_frame + 1) * 2, (s_battle_frame + 1) * 2), s_battle_frame == 8);
        s_battle_frame++;
      } else {
        s_battle_frame = 0;
        s_battle_state = PB_LOAD;
      }
      break;
    case PB_ANIM_DISTORT: // Distort screen
      if (s_battle_frame == 0) {
        GBC_Graphics_set_line_compare_interrupt_callback(graphics, distortion_callback);
        GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, true);
        GBC_Graphics_stat_set_line_y_compare(graphics, 0);
        s_battle_frame++;
      } else if (s_battle_frame == 10) {
        GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
        s_battle_frame = 0;
        GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
        draw_black_rectangle(graphics, GRect(0, 0, 18, 18), true);
        s_battle_state = PB_LOAD;
      } else {
        GBC_Graphics_stat_set_line_y_compare(graphics, 0);
        s_battle_frame++;
      }
      break;
    case PB_ANIM_RADIAL: { // Radial Wipe clockwise from 9 o'clock
      // While drawing one line per frame is very clean, we gotta speed it up
      // Thus, draw four lines per frame instead
      for (uint8_t i = 0; i < 4; i++) {
        int angle = TRIG_MAX_ANGLE * s_battle_frame / 128 ;
        
        uint8_t center_x, center_y;
        if (s_battle_frame < 32 || s_battle_frame >= 96) {
          center_x = 8;
        } else {
          center_x = 9;
        }
        if (s_battle_frame < 64) {
          center_y = 8;
        } else {
          center_y = 9;
        }
        int end_x = clamp((-cos_lookup(angle) * 12 / TRIG_MAX_RATIO) + center_x, 0, 17);
        int end_y = clamp((-sin_lookup(angle) * 12 / TRIG_MAX_RATIO) + center_y, 0, 17);
        draw_line(graphics, center_x, center_y, end_x, end_y);

        // Make up for the lines that the algorithm fails to hit
        if (s_battle_frame == 16) {
          draw_line(graphics, center_x, center_y, 0, 0);
        }
        if (s_battle_frame == 48) {
          draw_line(graphics, center_x, center_y, 17, 0);
        }
        if (s_battle_frame == 80) {
          draw_line(graphics, center_x, center_y, 17, 17);
        }
        if (s_battle_frame == 112) {
          draw_line(graphics, center_x, center_y, 0, 17);
        }
        s_battle_frame ++;
      }
      if (s_battle_frame == 128) {
        s_battle_frame = 0;
        draw_black_rectangle(graphics, GRect(0, 0, 18, 18), true);
        s_battle_state = PB_LOAD;
      }
    } break;
    case PB_ANIM_FADE: // Random squares
      if (s_battle_frame <= 32) {
        for (uint8_t i = 0; i < 16; i++) {
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
      for (uint8_t i = 0; i < 40; i++) {
        GBC_Graphics_oam_hide_sprite(graphics, i);
      }
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
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "Player Pokemon:\n\tName: %s\n\tLevel: %d\n\tHP: %d\n\tAttack: %d\n\tDefense: %d\n\t",
      //         s_player_pokemon_name, s_player_pokemon_level, s_player_pokemon_health, s_player_pokemon_attack, s_player_pokemon_defense);
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "Enemy Pokemon:\n\tName: %s\n\tLevel: %d\n\tHP: %d\n\tAttack: %d\n\tDefense: %d\n\t",
      //         s_enemy_pokemon_name, s_enemy_pokemon_level, s_enemy_pokemon_health, s_enemy_pokemon_attack, s_enemy_pokemon_defense);
      s_eaten_berry = false;

      // And set up for scroling
      GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, true);
      GBC_Graphics_set_line_compare_interrupt_callback(graphics, handle_battle_scroll_interrupt);
      GBC_Graphics_stat_set_line_y_compare(graphics, 0);
      s_battle_enemy_scroll_x = 144;
      s_battle_player_scroll_x = 112;

      s_escape_odds = rand() % 32;
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
      draw_exp_bar(graphics, s_to_next_level - s_to_cur_level, s_player_exp - s_to_cur_level);
      char level_text[8];
      if (s_enemy_pokemon_level < 100) {
        snprintf(level_text, 8, "|%-2d%c", s_enemy_pokemon_level, rand()%2 ? '^' : '+');
      } else {
        snprintf(level_text, 8, "%d%c", s_enemy_pokemon_level, rand()%2 ? '^' : '+');
      }
      draw_uint_string_at_location(graphics, GPoint(1, PBL_IF_ROUND_ELSE(1, 0)), s_enemy_pokemon_name, 11);
      draw_text_at_location(graphics, GPoint(4, PBL_IF_ROUND_ELSE(2, 1)), level_text);
      draw_uint_string_at_location(graphics, GPoint(8, 7), s_player_pokemon_name, 11);
      if (s_player_level < 100) {
        snprintf(level_text, 8, "|%-2d%c", s_player_level, rand()%2 ? '^' : '+');
      } else {
        snprintf(level_text, 8, "%d%c", s_player_level, rand()%2 ? '^' : '+');
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
      s_player_pokemon_damage = calculate_damage(s_player_level, s_player_pokemon_attack, s_enemy_pokemon_defense);
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
          s_battle_state = s_player_goes_first ? PB_ENEMY_MOVE : PB_LEFTOVERS;
          s_battle_frame = 0;
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
          if (s_player_pokemon_health == 1 && HAS_ITEM(s_player_items, ITEM_ID_FOCUS_BAND) && (rand() % 12 == 0)) {
            char focus_dialogue[40];
            snprintf(focus_dialogue, 40, "%s\nhung on with\nFOCUS BAND!", s_player_pokemon_name);
            begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, focus_dialogue, true);
            s_prev_game_state = PG_BATTLE;
            s_game_state = PG_DIALOGUE;
            s_enemy_pokemon_damage = 0;
          }
          if (s_player_pokemon_health == 0) {
            s_battle_state = PB_ENEMY_WIN;
            s_battle_frame = 0;
          }
          draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
        } else {
          if (HAS_ITEM(s_player_items, ITEM_ID_BERRY) && !s_eaten_berry && s_player_pokemon_health <= (s_player_max_pokemon_health / 2)) {
            s_eaten_berry = true;
            s_battle_state = PB_BERRY;
            s_battle_frame = 0;
          } else {
            s_battle_state = s_player_goes_first ? PB_LEFTOVERS : PB_PLAYER_MOVE;
            s_battle_frame = 0;
          }
        }
      }
      break;
    case PB_BERRY:
      if (s_battle_frame == 0) {
        char berry_dialogue[40];
        snprintf(berry_dialogue, 40, "%s ate\nits BERRY!", s_player_pokemon_name);
        s_clear_dialogue = false;
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, berry_dialogue, false);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_health_to_gain = 10;
        s_battle_frame++;
      } else {
        if (s_health_to_gain > 0 && s_player_pokemon_health < s_player_max_pokemon_health) {
          s_health_to_gain -= 1;
          s_player_pokemon_health += 1;
          draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
        } else {
          s_battle_frame = 13;
          s_battle_state = PB_ENEMY_EFFECT;
        }
      }
      break;
    case PB_LEFTOVERS:
      if (HAS_ITEM(s_player_items, ITEM_ID_LEFTOVERS)) {
        if (s_battle_frame == 0) {
          char leftover_dialogue[40];
          snprintf(leftover_dialogue, 40, "%s ate\nsome LEFTOVERS!", s_player_pokemon_name);
          s_clear_dialogue = false;
          begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, leftover_dialogue, false);
          s_prev_game_state = PG_BATTLE;
          s_game_state = PG_DIALOGUE;
          s_health_to_gain = s_player_max_pokemon_health / 16;
          s_battle_frame++;
        } else {
          if (s_health_to_gain > 0 && s_player_pokemon_health < s_player_max_pokemon_health) {
            s_health_to_gain -= 1;
            s_player_pokemon_health += 1;
            draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
          } else {
            s_battle_state = PB_PLAYER_TURN_PROMPT;
            s_battle_frame = 0;
          }
        }
      } else {
        s_battle_state = PB_PLAYER_TURN_PROMPT;
        s_battle_frame = 0;
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
        if (s_player_level < 100) {
          s_battle_state = PB_EXPERIENCE;
        } else {
          s_battle_state = PB_FADEOUT;
        }
        s_battle_frame = 0;
        s_stats_wins += 1;
        s_stats_battles += 1;
      }
    } break;
    case PB_EXPERIENCE:
      if (s_battle_frame == 0) {
        s_exp_gained = calculate_exp_gain(s_enemy_pokemon_level, HAS_ITEM(s_player_items, ITEM_ID_LUCKY_EGG));
        char exp_dialogue[40];
        snprintf(exp_dialogue, 40, "Gained %d EXP!", s_exp_gained);
        begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, exp_dialogue, true);
        s_prev_game_state = PG_BATTLE;
        s_game_state = PG_DIALOGUE;
        s_battle_frame++;
      } else {
        if (s_exp_gained > 0) {
          int bar_len_start = (s_player_exp << 6) / s_to_next_level; // "<< 6" == "* 64" but faster
          while (!(s_exp_gained <= 0 || (s_player_exp << 6) / s_to_next_level > bar_len_start || s_player_exp == s_to_next_level)) {
            s_exp_gained--;
            s_player_exp++;
          }
          if (s_player_exp == s_to_next_level) {
            s_player_level++;
            draw_player_battle_frame(graphics);
            draw_player_hp_bar(graphics, s_player_max_pokemon_health, s_player_pokemon_health);
            char level_text[4];
            if (s_player_level < 100) {
              snprintf(level_text, 4, "|%-2d", s_player_level);
            } else {
              snprintf(level_text, 4, "%d", s_player_level);
            }
            draw_text_at_location(graphics, GPoint(12, 8), level_text);
            s_to_cur_level = s_to_next_level;
            s_to_next_level = cube(s_player_level+1);
            char exp_dialogue[40];
            snprintf(exp_dialogue, 40, "Lvl grew to %d!%s", s_player_level, (s_player_level % 5 == 0) ? "\n\nSprite unlocked!" : "");
            begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, exp_dialogue, true);
            s_prev_game_state = PG_BATTLE;
            s_game_state = PG_DIALOGUE;
            draw_exp_bar(graphics, s_to_cur_level, s_player_exp);
            if (s_player_level == 100) {
              s_battle_state = PB_FADEOUT;
              s_battle_frame = 0;
            }
          } else {
            draw_exp_bar(graphics, s_to_next_level - s_to_cur_level, s_player_exp - s_to_cur_level);
          }
        } else {
          s_battle_state = PB_FADEOUT;
          s_battle_frame = 0;
        }
      }
      break;
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
      if (s_escape_odds != 0) {
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
    .player_sprite = s_player_sprite,
    .move_mode_toggle = s_move_mode_toggle,
    .turn_mode_tilt = s_turn_mode_tilt,
    .backlight_on = s_backlight_on,
    .text_speed = s_text_speed,
    .player_level = s_player_level,
    .player_exp = s_player_exp,
    .last_save = time(NULL),
    .battles = s_stats_battles,
    .wins = s_stats_wins,
    .losses = s_stats_losses,
    .runs = s_stats_runs,
    .player_items = s_player_items,
  };
  persist_write_data(SAVE_KEY, &data, sizeof(PokemonSaveData));
  s_save_file_exists = true;
}

void Pokemon_step(GBC_Graphics *graphics) {
  s_step_frame = (s_step_frame + 1) % FRAME_SKIP;
  if (s_step_frame != 0) {
      return;
  }
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
        step_dialogue(graphics, s_select_pressed, s_text_speed);
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
        } else if (s_prev_game_state == PG_TREE) {
          switch (s_tree_state) {
            case PT_CONFIRM:
              set_cursor_pos(0);
              draw_menu(graphics, CONFIRM_BOUNDS, "YES\n\nNO", false, true);
              set_num_menu_items(2);
              s_game_state = PG_TREE;
              s_select_pressed = false;
              break;
            case PT_REPLACE:
              load_screen(graphics);
              s_game_state = PG_TREE;
              s_select_pressed = false;
              break;
            default:
              break;
          }
        }
      }
      break;
    case PG_BATTLE:
      battle(graphics);
      break;
    case PG_TREE:
      switch (s_tree_state) {
        case PT_CONFIRM:
          break;
        case PT_REPLACE: {
          uint16_t target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
          uint16_t target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
          set_block(s_world_map, target_x, target_y, replacement_blocks[s_route_num]);
          load_screen(graphics);
          s_tree_state = PT_ANIMATE;
          int x_diff = target_x - s_player_x;
          int y_diff = target_y - s_player_y;

          GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 0, x_diff + 72, y_diff + 72 + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 1, x_diff + 72 + 8, y_diff + 72 + 8);
          GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 2, x_diff + 72, y_diff + 72 + 16);
          GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 3, x_diff + 72 + 8, y_diff + 72 + 16);
          s_tree_frame = 0;
        } break;
        case PT_ANIMATE:
          switch (s_tree_frame) {
            case 0:
              GBC_Graphics_oam_move_sprite(graphics, 24 + 0, -2, 0);
              GBC_Graphics_oam_move_sprite(graphics, 24 + 1, 2, 0);
              GBC_Graphics_oam_move_sprite(graphics, 24 + 2, -2, 0);
              GBC_Graphics_oam_move_sprite(graphics, 24 + 3, 2, 0);
              break;
            case 8:
              s_game_state = PG_PLAY;
            case 4:
            case 6:
              GBC_Graphics_oam_hide_sprite(graphics, 24 + 0);
              GBC_Graphics_oam_hide_sprite(graphics, 24 + 1);
              GBC_Graphics_oam_hide_sprite(graphics, 24 + 2);
              GBC_Graphics_oam_hide_sprite(graphics, 24 + 3);
              break;
            case 5: {
              uint16_t target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
              uint16_t target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
              int x_diff = target_x - s_player_x;
              int y_diff = target_y - s_player_y;

              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 0, x_diff + 72 - 6, y_diff + 72 + 8);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 1, x_diff + 72 + 12, y_diff + 72 + 8);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 2, x_diff + 72 - 6, y_diff + 72 + 16);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 3, x_diff + 72 + 12, y_diff + 72 + 16);
            } break;
            case 7: {
              uint16_t target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
              uint16_t target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
              int x_diff = target_x - s_player_x;
              int y_diff = target_y - s_player_y;

              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 0, x_diff + 72 - 10, y_diff + 72 + 8);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 1, x_diff + 72 + 16, y_diff + 72 + 8);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 2, x_diff + 72 - 10, y_diff + 72 + 16);
              GBC_Graphics_oam_set_sprite_pos(graphics, 24 + 3, x_diff + 72 + 16, y_diff + 72 + 16);
            } break;
          }
          s_tree_frame++;
          break;
      }
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

static void fill_str(char *str, char fill, uint8_t num) {
  for (uint8_t i = 0; i < num; i++) {
    str[i] = fill;
  }
}

static void set_stat(char *buffer, char *str, int data) {
  fill_str(buffer, '.', 14);
  memcpy(buffer, str, strlen(str));
  char data_buffer[14] = {0};
  snprintf(data_buffer, 14, "%d", data);
  memcpy(buffer+(strlen(buffer)-strlen(data_buffer)), data_buffer, strlen(data_buffer));
}

static void draw_stats(GBC_Graphics *graphics) {
  char text_buffer[20] = {0};
  draw_menu_rectangle(graphics, GRect(STATS_ROOT_X, STATS_ROOT_Y, 18, 18));

  set_stat(text_buffer, "Level", s_player_level);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 3), text_buffer);

  set_stat(text_buffer, "EXP", s_player_exp);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 5), text_buffer);

  set_stat(text_buffer, "Lvl up", s_to_next_level - s_player_exp);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 7), text_buffer);

  set_stat(text_buffer, "Battles", s_stats_battles);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 9), text_buffer);

  set_stat(text_buffer, "Wins", s_stats_wins);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 11), text_buffer);

  set_stat(text_buffer, "Losses", s_stats_losses);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 13), text_buffer);

  set_stat(text_buffer, "Runs", s_stats_runs);
  draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 15), text_buffer);
}

static void draw_items(GBC_Graphics *graphics) {
  draw_menu_rectangle(graphics, GRect(STATS_ROOT_X, STATS_ROOT_Y, 18, 18));
  for (uint8_t i = 0; i < 8; i++) {
    if (HAS_ITEM(s_player_items, i)) {
      draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 1 + i*2), item_names[i]);
      draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 1 + i*2 + 1), item_infos[i]);
    } else {
      draw_text_at_location(graphics, GPoint(STATS_ROOT_X + 2, STATS_ROOT_Y + 1 + i*2), "______________");
    }
  }
}

void Pokemon_handle_select_click(GBC_Graphics *graphics) {
  switch(s_game_state) {
    case PG_PAUSE:
      switch(s_menu_state) {
        case PM_BASE:
          switch (get_cursor_pos()) {
            case 0: // Stats
              draw_stats(graphics);
              s_menu_state = PM_STATS;
              break;
            case 1: // Pebble
              draw_pebble_info(graphics);
              s_menu_state = PM_PEBBLE;
              break;
            case 2: // Option
              set_cursor_pos(0);
              draw_option_menu(graphics);
              s_menu_state = PM_OPTION;
              break;
            case 3: // Pack
              draw_items(graphics);
              s_menu_state = PM_PACK;
              break;
            case 4: // Quit
              window_stack_pop(true);
              break;
          }
          break;
        case PM_OPTION:
          switch (get_cursor_pos()) {
            case 0: // Move Mode
              s_move_mode_toggle = !s_move_mode_toggle;
              draw_option_menu(graphics);
              break;
            case 1: // Turn Mode
              s_turn_mode_tilt = !s_turn_mode_tilt;
              AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
              accel_service_peek(&accel);
              s_accel_x_cal = accel.x;
              s_accel_y_cal = accel.y;
              draw_option_menu(graphics);
              break;
            case 2: // Text Speed
              s_text_speed = (s_text_speed + 1) % 3;
              draw_option_menu(graphics);
              break;
            case 3: // Backlight
              s_backlight_on = !s_backlight_on;
              light_enable(s_backlight_on);
              draw_option_menu(graphics);
              break;
            case 4: // Sprite
              s_player_sprite = (s_player_sprite + 1) % (2 + (s_player_level / 5));
              load_player_sprites(graphics);
              draw_option_menu(graphics);
              break;
            case 5: // Cancel
              s_menu_state = PM_BASE;
              set_cursor_pos(2);
              hide_preview_sprites(graphics, 8);
              draw_pause_menu(graphics);
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
        case PM_PACK:
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
            s_player_sprite = data.player_sprite;
            s_move_mode_toggle = data.move_mode_toggle;
            s_turn_mode_tilt = data.turn_mode_tilt;
            s_backlight_on = data.backlight_on;
            s_text_speed = data.text_speed;
            s_player_level = data.player_level;
            s_to_next_level = cube(s_player_level+1);
            s_to_cur_level = s_player_level == 1 ? 0 : cube(s_player_level);
            s_player_exp = data.player_exp;
            s_stats_battles = data.battles;
            s_stats_wins = data.wins;
            s_stats_losses = data.losses;
            s_stats_runs = data.runs;
            s_player_items = data.player_items;
            load_game(graphics);
            s_game_state = PG_PLAY;
            s_select_pressed = false;
          } break;
          case 1: // New
            s_player_x = PLAYER_ORIGIN_X;
            s_player_y = PLAYER_ORIGIN_Y;
            new_player_sprites(graphics);
            load_game(graphics);
            s_to_next_level = cube(s_player_level+1);
            s_to_cur_level = s_player_level == 1 ? 0 : cube(s_player_level);
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
              s_player_goes_first = rand() % 2;
              s_battle_state = s_player_goes_first ? PB_PLAYER_MOVE : PB_ENEMY_MOVE;
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
    case PG_PLAY:
      if (s_move_mode_toggle) {
        s_move_toggle = !s_move_toggle;
      }
      break;
    case PG_TREE:
      switch (s_tree_state) {
        case PT_CONFIRM:
          switch (get_cursor_pos()) {
            case 0: // Yes
              s_select_pressed = false;
              s_prev_game_state = PG_TREE;
              s_tree_state = PT_REPLACE;
              s_game_state = PG_DIALOGUE;
              load_screen(graphics);
              begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, "You used CUT!", true);
              break;
            case 1: // No
              s_select_pressed = false;
              load_screen(graphics);
              s_game_state = PG_PLAY;
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
      s_down_press_queued = true;
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_down(graphics);
          draw_menu_info(graphics);
          break;
        case PM_OPTION:
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
    case PG_TREE:
      switch (s_tree_state) {
        case PT_CONFIRM:
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
      s_up_press_queued = true;
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_up(graphics);
          draw_menu_info(graphics);
          break;
        case PM_OPTION:
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
    case PG_TREE:
      switch (s_tree_state) {
        case PT_CONFIRM:
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
  // if (s_game_state == PG_PLAY) {
  //   new_player_sprites(graphics);
  // }
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
      if (s_move_mode_toggle) {
        s_move_toggle = false;
      }
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
        case PM_OPTION:
          s_menu_state = PM_BASE;
          set_cursor_pos(2);
          hide_preview_sprites(graphics, 8);
          draw_pause_menu(graphics);
          break;
        case PM_PACK:
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

void Pokemon_deinitialize(GBC_Graphics *graphics) {
  save();
  unload_dialogue();
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
  layer_set_update_proc(s_background_layer, NULL);
}