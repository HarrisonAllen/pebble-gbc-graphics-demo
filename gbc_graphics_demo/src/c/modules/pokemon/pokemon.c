#include "pokemon.h"
#include "menu.h"
#include "world.h"
#include "sprites.h"
#include "objects.h"

typedef enum {
  D_UP,
  D_LEFT,
  D_DOWN,
  D_RIGHT
} Dir;

typedef enum {
  P_STAND,
  P_WALK
} PlayerMode;

typedef enum {
  PG_INTRO,
  PG_PLAY,
  PG_PAUSE_QUEUED,
  PG_PAUSE,
  PG_DIALOGUE
} PokemonGameState;

typedef enum {
  PM_BASE,
  PM_PEBBLE,
  PM_SAVE,
  PM_SAVE_CONFIRM,
} PokemonMenuState;

static Dir s_player_direction;
static PlayerMode s_player_mode;
static uint8_t s_walk_frame, s_poll_frame, s_dialogue_frame;
static bool s_select_pressed, s_flip_walk;
static uint16_t s_target_x, s_target_y;
static bool s_can_move;
static uint8_t *s_world_map;
static uint16_t s_player_x, s_player_y;
static uint8_t s_player_sprite_x, s_player_sprite_y;
static Layer *s_background_layer;
static const uint8_t *s_player_palette;
static PokemonGameState s_game_state;
static void (*s_next_demo_callback)();
static PokemonMenuState s_menu_state;
static bool s_save_file_exists;

static GPoint direction_to_point(Dir dir) {
    switch (dir) {
        case D_UP:  return GPoint(0, -1);
        case D_LEFT:  return GPoint(-1, 0);
        case D_DOWN:  return GPoint(0, 1);
        case D_RIGHT:  return GPoint(1, 0);
        default: return GPoint(0, 0);
    }
}

/*
 * Todo: Menu
 * Note that for pokemon, the process is:
 * 1. Copy the current bg to the window layer (screen only)
 * 2. Hide all the sprites underneath the menu (with my implementation we can use the bg priority bit)
 * 3. Copy the window layer back onto the background at 0,0
 * 3. Slap the menu on top of the background
 * 
 * Menu items:
 *  - Resume (close menu)
 *  - Watch Info
 *    - Time, battery, date
 *  - Mario
 *  - Save
 *  - Quit
 */

/*
 * Todo: Battle
 * For Pokemon, the process is:
 * 1. Shift the palettes between black and white
 * 2. Draw a circle radius from 180 deg -> 180 deg clockwise, by setting tiles to black
 * 3. Screen is now black, load in the trainer's hat and the enemy pokemon (and animation)
 * 4. Draw the pokemon sprite at (0D, 01), vertically (6x6, no compression!)
 * 5. Draw the player at (02, 06), but the top two rows are sprites
 * 6. Now we do the HBlank magic, where:
 *    - The enemy pokemon comes in from the left
 *    - The player comes from the right (with top 2 rows of sprites)
 *    - And the menu bar thing goes nowhere
 *    - Everything is black and white for some reason
 * 7. "A wild ______ appeared! v"
 * 8. Enemy pokemon health bar and name and level and gender come up (01, 00)
 * 9. Player slides off-screen (need to check this if there are sprites or smth)
 *    - Replace player tiles with player pokemon
 * 10. "Go! _______!"
 * 11. Pokeball pops open
 * 12. Player pokemon appears under it, basically start loading tiles from the bottom
 * 13. Player pokemon stats appear (09, 07)
 * 14. Action menu pops up (Fight, Run)
 * 15. Run:
 *    1. "Got away safely! v"
 * 16. Fight:
 *    1. Pick a move
 *    2. "______ used _______!"
 *    3. Player pokemon stats disappear
 *    4. Attack animation appears as sprites
 *    5. Enemy pokemon flashes (tile swapping)
 *    6. Enemy health goes down (each bar is its own tile!)
 *    7. Repeat 2-6 for the enemy pokemon
 *    8. Enemy pokemon faints, moves downwards
 *    9. "Enemy _______ fainted! v"
 *    10. "______ gained ______ exp points! v"
 *    11. 
 * 16. Fade to white
 * 17. Load background from window
 * 18. Unfade
 */

/*
 * TODO: Add some animated tiles
 */

static uint8_t get_tile_id_from_map(uint8_t *map, uint16_t tile_x, uint16_t tile_y) {
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * POKEMON_MAP_CHUNK_WIDTH];
  uint8_t block = pokemon_chunks[chunk][((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return pokemon_blocks[block][(tile_x & 1) + (tile_y & 1) * 2];
}

static void load_tiles(GBC_Graphics *graphics, uint8_t bg_root_x, uint8_t bg_root_y, uint16_t tile_root_x, uint16_t tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
  for (uint16_t tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
    for (uint16_t tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
      uint8_t tile = get_tile_id_from_map(s_world_map, tile_x, tile_y);
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, pokemon_tile_palettes[tile]);
    }
  }
}

static void load_tiles_to_window(GBC_Graphics *graphics, uint8_t bg_root_x, uint8_t bg_root_y, uint16_t tile_root_x, uint16_t tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
  for (uint16_t tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
    for (uint16_t tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
      uint8_t tile = get_tile_id_from_map(s_world_map, tile_x, tile_y);
      GBC_Graphics_window_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile, pokemon_tile_palettes[tile]);
    }
  }
}

static void load_screen(GBC_Graphics *graphics) {
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
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  #if defined(PBL_BW)
    // Do some basic dithering
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    for (uint8_t y = 0; y < bounds.size.h/2; y++) {
      GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
      for (uint8_t x = 0; x < bounds.size.w; x++) {
        uint8_t pixel_color = ((x + (y & 1)) & 1);
        uint16_t byte = (x >> 3); // x / 8
        uint8_t bit = x & 7; // x % 8
        uint8_t *byte_mod = &info.data[byte];
        *byte_mod ^= (-pixel_color ^ *byte_mod) & (1 << bit);
      }
    }
    graphics_release_frame_buffer(ctx, fb);
  #endif

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

static void load_player_sprites(GBC_Graphics *graphics) {
  uint8_t sprites_to_use = rand() % 8;
  uint8_t palette_to_use = rand() % 6;

  s_player_palette = pokemon_trainer_sprite_palettes[palette_to_use];
  GBC_Graphics_set_sprite_palette(graphics, 0, s_player_palette[0], s_player_palette[1], s_player_palette[2], s_player_palette[3]);

  uint16_t spritesheet_offset = pokemon_trainer_sheet_offsets[sprites_to_use] * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_SPRITESHEET,
    spritesheet_offset, POKEMON_TILES_PER_SPRITE * POKEMON_SPRITES_PER_TRAINER, 1, 3); // offset 1 for grass
}

static void set_player_sprites(GBC_Graphics *graphics, bool walk_sprite, bool x_flip) {
  uint8_t new_tile = pokemon_trainer_sprite_offsets[s_player_direction + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, 2 + i, (new_tile * POKEMON_TILES_PER_SPRITE) + i + 1);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, 2 + i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 2, 3);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 4, 5);
  }
}

static bool load(time_t *last_save, uint16_t *player_x, uint16_t *player_y) {
  PokemonSaveData data;
  bool save_exists = false;
  if (persist_read_data(POKEMON_SAVE_KEY, &data, sizeof(PokemonSaveData)) != E_DOES_NOT_EXIST) {
    if (player_x)
      *player_x = data.player_x;
    if (player_y)
      *player_y = data.player_y;
    if (last_save)
      *last_save = data.last_save;
    save_exists = true;
  }
  return save_exists;
}

static uint8_t get_block_type(uint8_t *map, uint16_t x, uint16_t y) {
  uint16_t tile_x = (x >> 3);
  uint16_t tile_y = (y >> 3);
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * POKEMON_MAP_CHUNK_WIDTH];
  uint8_t block = pokemon_chunks[chunk][((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return pokemon_block_type[block];
}

static void load_game(GBC_Graphics *graphics) {
  s_player_direction = D_DOWN;
  s_player_mode = P_STAND;
  s_game_state = PG_PLAY;
  s_walk_frame = 0;
  s_poll_frame = 0;
  s_player_sprite_x = 16 * 4 + SPRITE_OFFSET_X;
  s_player_sprite_y = 16 * 4 + SPRITE_OFFSET_Y;

  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_TILESHEET, 0, 19, 0, 0);
  
  ResHandle handle = resource_get_handle(POKEMON_MAP_FILE);
  size_t res_size = resource_size(handle);
  s_world_map = (uint8_t*)malloc(res_size);
  resource_load(handle, s_world_map, res_size);

  load_screen(graphics);

  load_player_sprites(graphics);

  // Create grass effect sprites
  uint16_t spritesheet_offset = POKEMON_SPRITELET_GRASS * POKEMON_TILES_PER_SPRITE;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_SPRITESHEET, spritesheet_offset, 1, 0, 3);
  GBC_Graphics_oam_set_sprite(graphics, 0, 0, 0, 0, GBC_Graphics_attr_make(6, 3, false, false, false));
  GBC_Graphics_oam_set_sprite(graphics, 1, 0, 0, 0, GBC_Graphics_attr_make(6, 3, true, false, false));
  #if defined(PBL_COLOR)
    GBC_Graphics_set_sprite_palette(graphics, 6, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
  #else
    GBC_Graphics_set_sprite_palette(graphics, 6, 1, 1, 0, 0);
  #endif

  // Crate player sprites
  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      GBC_Graphics_oam_set_sprite_pos(graphics, 2 + x + 2 * y, s_player_sprite_x + x * 8, s_player_sprite_y + y * 8 - 4);
      GBC_Graphics_oam_set_sprite_attrs(graphics, 2 + x + 2 * y, GBC_Graphics_attr_make(0, 3, false, false, false));
    }
  }
  set_player_sprites(graphics, false, false);

  if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
#if defined(PBL_COLOR)
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
#else
    GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
    GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
#endif
  }

  GBC_Graphics_render(graphics);
}

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer, void (*next_demo_callback)()) {
  layer_set_update_proc(background_layer, background_update_proc);
  s_background_layer = background_layer;
  
  s_next_demo_callback = next_demo_callback;

  s_game_state = PG_INTRO;
  GBC_Graphics_set_screen_bounds(graphics, SCREEN_BOUNDS_SQUARE);
  GBC_Graphics_window_set_offset_pos(graphics, 0, 168);
  GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, false);
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

#if defined(PBL_COLOR)
  GBC_Graphics_set_bg_palette(graphics, 0, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 1, 0b11000000, 0b11000000, 0b11000000, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 2, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 3, 0b11000000, 0b11000000, 0b11000000, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 4, 0b11000000, 0b11000000, 0b11000000, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 5, 0b11111111, 0b11111001, 0b11100100, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 6, 0b11000000, 0b11000000, 0b11000000, 0b11000000);
  GBC_Graphics_set_bg_palette(graphics, 7, 0b11000000, 0b11000000, 0b11000000, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, 0, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 1, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 2, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 3, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 4, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 5, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 6, 0, 0, 1, 1);
  GBC_Graphics_set_bg_palette(graphics, 7, 0, 0, 1, 1);
#endif

  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_MENU_TILESHEET, 0, 104, 0, 1);

  GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
  GBC_Graphics_lcdc_set_bg_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_sprite_layer_enabled(graphics, true);

  time_t last_save;
  set_cursor_pos(0);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);

  draw_blank_rectangle(graphics, GRect(0, 0, 18, 18));
  if (load(&last_save, NULL, NULL)) {
    s_save_file_exists = true;

    draw_menu(graphics, GRect(POKEMON_START_MENU_ROOT_X, POKEMON_START_MENU_ROOT_Y, 14, 8), "CONTINUE\n\nNEW GAME\n\nQUIT", false);
    set_num_menu_items(3);

    draw_textbox(graphics, GRect(POKEMON_START_INFO_ROOT_X, POKEMON_START_INFO_ROOT_Y, 16, 5), GPoint(1, 1), "Last save:", false);
    struct tm *tick_time = localtime(&last_save);
    char text_buffer[20] = {0};
    strftime(text_buffer, sizeof(text_buffer), "%a %b %d", tick_time);
    draw_text_at_location(graphics, GPoint(POKEMON_START_INFO_ROOT_X + 3, POKEMON_START_INFO_ROOT_Y + 2), text_buffer);
    strftime(text_buffer, sizeof(text_buffer), "%r", tick_time);
    draw_text_at_location(graphics, GPoint(POKEMON_START_INFO_ROOT_X + 3, POKEMON_START_INFO_ROOT_Y + 3), text_buffer);
  } else {    
    draw_menu(graphics, GRect(POKEMON_START_MENU_ROOT_X, POKEMON_START_MENU_ROOT_Y, 12, 6), "NEW GAME\n\nQUIT", false);
    set_num_menu_items(2);
  }

  GBC_Graphics_render(graphics);

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

static void load_blocks_in_direction(GBC_Graphics *graphics, Dir direction) {
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

  for (uint8_t i = 0; i < sizeof(dialogue_objects); i++) {
    if (block_x == dialogue_objects[i][0] && block_y == dialogue_objects[i][1]) {
      return dialogue_objects[i][2];
    }
  }
  return -1;
}

static void play(GBC_Graphics *graphics) {
  if (s_select_pressed && s_player_mode == P_STAND) {
    s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
    s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
    
    int dialogue_id = check_for_object(s_target_x+8, s_target_y);
    if (dialogue_id != -1) {
      load_screen(graphics);
      begin_dialogue(graphics, POKEMON_DIALOGUE_BOUNDS, GPoint(1, 14), dialogue_id);
      s_game_state = PG_DIALOGUE;
      s_can_move = false;
      s_target_x = s_player_x;
      s_target_y = s_player_y;
      GBC_Graphics_render(graphics);
    } else {
      s_player_mode = P_WALK;
      s_walk_frame = 0;
      s_flip_walk = !s_flip_walk;
      GBC_Graphics_oam_set_sprite_priority(graphics, 4, false);
      GBC_Graphics_oam_set_sprite_priority(graphics, 5, false);
      
      s_can_move = get_block_type(s_world_map, s_target_x+8, s_target_y) != BLOCK;

      if (!s_can_move) {
        s_target_x = s_player_x;
        s_target_y = s_player_y;
      } else {
        load_blocks_in_direction(graphics, s_player_direction);
      }
    #if defined(PBL_BW)
      GBC_Graphics_oam_hide_sprite(graphics, 0);
      GBC_Graphics_oam_hide_sprite(graphics, 1);
    #endif
      GBC_Graphics_render(graphics);
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
        if (s_walk_frame % FPS_DELAY == 0) {
          s_player_x += direction_to_point(s_player_direction).x * 2;
          s_player_y += direction_to_point(s_player_direction).y * 2;
          GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
        }
      }
      if (get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS) { // grass
        switch(s_walk_frame) {
          case (int)(2 * FPS_DELAY):
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x, s_player_sprite_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 8, s_player_sprite_y + 4);
            break;
          case (int)(5 * FPS_DELAY):
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_sprite_x - 1, s_player_sprite_y + 5);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_sprite_x + 9, s_player_sprite_y + 5);
            break;
          case (int)(7 * FPS_DELAY):
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
      switch(s_walk_frame) {
        case (int)(7 * FPS_DELAY):
          s_player_mode = P_STAND;
        #if defined(PBL_COLOR)
          if(get_block_type(s_world_map, s_player_x+8, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        #endif
          break;
        case 0:
        case (int)(6 * FPS_DELAY):
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case (int)(2 * FPS_DELAY):
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      s_walk_frame++;
      GBC_Graphics_render(graphics);
      break;
    default:
      break;
  }
}

static void draw_menu_info(GBC_Graphics *graphics) {
  switch (get_cursor_pos()) {
    case 0:
      draw_textbox(graphics, GRect(POKEMON_INFO_ROOT_X, POKEMON_INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Close this\n\nmenu", true);
      break;
    case 1:
      draw_textbox(graphics, GRect(POKEMON_INFO_ROOT_X, POKEMON_INFO_ROOT_Y, 14, 5), GPoint(1, 1), "View time &\n\nPebble info", true);
      break;
    case 2:
      draw_textbox(graphics, GRect(POKEMON_INFO_ROOT_X, POKEMON_INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Switch to\n\nnext demo", true);
      break;
    case 3:
      draw_textbox(graphics, GRect(POKEMON_INFO_ROOT_X, POKEMON_INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Save your\n\nprogress", true);
      break;
    case 4:
      draw_textbox(graphics, GRect(POKEMON_INFO_ROOT_X, POKEMON_INFO_ROOT_Y, 14, 5), GPoint(1, 1), "Quit this\n\ndemo app", true);
      break;
    default:
      break;
  }
}

static void draw_pause_menu(GBC_Graphics *graphics) {
  load_screen(graphics);
  draw_menu(graphics, GRect(POKEMON_MENU_ROOT_X, POKEMON_MENU_ROOT_Y, 9, POKEMON_NUM_MENU_ITEMS * 2 + 2), "RESUME\n\nPEBBLE\n\nNEXT\n\nSAVE\n\nQUIT", false);
  set_num_menu_items(POKEMON_NUM_MENU_ITEMS);
  draw_menu_info(graphics);
}

void Pokemon_step(GBC_Graphics *graphics) {
  // s_poll_frame = (s_poll_frame + 1) % 8;
  // AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  // Dir old_direction = s_player_direction;
  if (s_player_mode == P_STAND && s_game_state == PG_PAUSE_QUEUED) {
    set_cursor_pos(0);
    draw_pause_menu(graphics);
    s_game_state = PG_PAUSE;
    s_menu_state = PM_BASE;
    GBC_Graphics_render(graphics);
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
        step_dialogue(graphics, s_select_pressed, FPS_DELAY);
      } else {
        load_screen(graphics);
        s_game_state = PG_PLAY;
        s_select_pressed = false;
        GBC_Graphics_render(graphics);
      }
    default:
      break;
  }
}

void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed) {
  s_select_pressed = pressed;
}

static void save() {
  PokemonSaveData data = (PokemonSaveData) {
    .player_x = s_player_x,
    .player_y = s_player_y,
    .last_save = time(NULL),
  };
  persist_write_data(POKEMON_SAVE_KEY, &data, sizeof(PokemonSaveData));
  s_save_file_exists = true;
}

static void draw_pebble_info(GBC_Graphics *graphics) {
  char text_buffer[20] = {0};
  // load_screen(graphics);
  draw_menu_rectangle(graphics, GRect(POKEMON_PEBBLE_ROOT_X, POKEMON_PEBBLE_ROOT_Y, 16, 11));
  
  char time_buffer[9] = {0};
  clock_copy_time_string(time_buffer, 9);
  draw_text_at_location(graphics, GPoint(POKEMON_PEBBLE_ROOT_X + 2, POKEMON_PEBBLE_ROOT_Y + 2), time_buffer);

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(text_buffer, sizeof(text_buffer), "%A", tick_time);
  draw_text_at_location(graphics, GPoint(POKEMON_PEBBLE_ROOT_X + 2, POKEMON_PEBBLE_ROOT_Y + 4), text_buffer);
  strftime(text_buffer, sizeof(text_buffer), "%b %d, %Y", tick_time);
  draw_text_at_location(graphics, GPoint(POKEMON_PEBBLE_ROOT_X + 2, POKEMON_PEBBLE_ROOT_Y + 6), text_buffer);

  snprintf(text_buffer, sizeof(text_buffer), "Battery: %d%%", battery_state_service_peek().charge_percent);
  draw_text_at_location(graphics, GPoint(POKEMON_PEBBLE_ROOT_X + 2, POKEMON_PEBBLE_ROOT_Y + 8), text_buffer);
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
              GBC_Graphics_render(graphics);
              break;
            case 1: // Pebble
              draw_pebble_info(graphics);
              s_menu_state = PM_PEBBLE;
              GBC_Graphics_render(graphics);
              break;
            case 2: // Next
              s_next_demo_callback();
              break;
            case 3: // Save
              save();
              break;
            case 4: // Quit
              window_stack_pop(true);
              break;
          }
          break;
        case PM_PEBBLE:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          GBC_Graphics_render(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      if (s_save_file_exists) {
        switch(get_cursor_pos()) {
          case 0: // Continue
            load(NULL, &s_player_x, &s_player_y);
            load_game(graphics);
            s_game_state = PG_PLAY;
            s_select_pressed = false;
            break;
          case 1: // New
            s_player_x = POKEMON_PLAYER_ORIGIN_X;
            s_player_y = POKEMON_PLAYER_ORIGIN_Y;
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
            s_player_x = POKEMON_PLAYER_ORIGIN_X;
            s_player_y = POKEMON_PLAYER_ORIGIN_Y;
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
        GBC_Graphics_render(graphics);
      }
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_down(graphics);
          draw_menu_info(graphics);
          GBC_Graphics_render(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      move_cursor_down(graphics);
      GBC_Graphics_render(graphics);
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
        GBC_Graphics_render(graphics);
      }
      break;
    case PG_PAUSE:
      switch (s_menu_state) {
        case PM_BASE:
          move_cursor_up(graphics);
          draw_menu_info(graphics);
          GBC_Graphics_render(graphics);
          break;
        default:
          break;
      }
      break;
    case PG_INTRO:
      move_cursor_up(graphics);
      GBC_Graphics_render(graphics);
      break;
    default:
      break;
  }
}

void Pokemon_handle_tap(GBC_Graphics *graphics) {
  if (s_game_state == PG_PLAY) {
    load_player_sprites(graphics);
    GBC_Graphics_render(graphics);
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
          GBC_Graphics_render(graphics);
          s_select_pressed = false;
          break;
        case PM_PEBBLE:
          s_menu_state = PM_BASE;
          draw_pause_menu(graphics);
          GBC_Graphics_render(graphics);
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