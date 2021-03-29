#include "pokemon.h"
#include "objects.h"
#include "object_attrs.h"
#include "screen.h"

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

static Dir s_player_direction;
static PlayerMode s_player_mode;
static uint8_t s_walk_frame, s_poll_frame;
static bool s_select_pressed, s_flip_walk;
static uint8_t s_player_x, s_player_y;
static uint8_t s_target_x, s_target_y;
static bool s_switch_character = true;
static const uint8_t *s_player_sprites;
static bool s_can_move;


static GPoint direction_to_point(Dir dir) {
    switch (dir) {
        case D_UP:  return GPoint(0, -1);
        case D_LEFT:  return GPoint(-1, 0);
        case D_DOWN:  return GPoint(0, 1);
        case D_RIGHT:  return GPoint(1, 0);
        default: return GPoint(0, 0);
    }
}

static uint8_t interp_int(uint8_t start, uint8_t end, uint8_t time, uint8_t time_max) {
    return start + (end - start) * (((float)time) / (float)time_max);
}

PokemonSquareInfo get_square_info(uint8_t x, uint8_t y) {
  return screen_info[x / (TILE_WIDTH * 2) + y / (TILE_WIDTH * 2) * (SCREEN_WIDTH / (TILE_WIDTH * 2))];
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
 *  - Tilt [ ] (this is a checkbox) (when tilt is turned on, also calibrate the accel)
 *  - Mario
 *  - Save
 *  - Load
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
 * TODO: Change the graphics to scrolling
 * Still fence the player in, but keep the player in the center, like in the game
 */

/*
 * TODO: Load in the tilemap from memory rather than having it stored in ram
 */

/*
 * TODO: Add some animated tiles
 */

void Pokemon_initialize(GBC_Graphics *graphics) {
    s_player_direction = D_DOWN;
    s_player_mode = P_STAND;
    s_walk_frame = 0;
    s_poll_frame = 0;
    s_player_x = 16 * 4 + 8;
    s_player_y = 16 * 5 + 8;
    s_player_sprites = s_switch_character ? kris_sprites : chris_sprites;

    // Load the sprites and tiles into separate banks, just to show it being done
    GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_TILESHEET, 49, 14, 0, 1);
    GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_POKEMON_TILESHEET, 0, 49, 0, 2);
    GBC_Graphics_lcdc_set_window_layer_enabled(graphics, false);
    GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, false);
    for (uint8_t i = 0; i < 40; i++) {
        GBC_Graphics_oam_hide_sprite(graphics, i);
    }

    GBC_Graphics_bg_set_scroll_pos(graphics, 0, 8);
    const uint8_t *object;
    const uint8_t *attrs;
    for (uint8_t y = 0; y < 11; y++) {
      for (uint8_t x = 0; x < 9; x++) {
        object = Pokemon_object_array[screen[x + y * 9]];
        attrs = Pokemon_attr_object_array[screen[x + y * 9]];

        GBC_Graphics_bg_set_tile_and_attrs(graphics, x * 2, y * 2, object[0], attrs[0]);
        GBC_Graphics_bg_set_tile_and_attrs(graphics, x * 2 + 1, y * 2, object[1], attrs[1]);
        GBC_Graphics_bg_set_tile_and_attrs(graphics, x * 2, y * 2 + 1, object[2], attrs[2]);
        GBC_Graphics_bg_set_tile_and_attrs(graphics, x * 2 + 1, y * 2 + 1, object[3], attrs[3]);
      }
    }
    GBC_Graphics_set_bg_palette(graphics, 0, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 2, 0b11101101, 0b11011000, 0b11000100, 0b11000000);
    GBC_Graphics_set_bg_palette(graphics, 5, 0b11111111, 0b11111001, 0b11100100, 0b11000000);

    // Create grass effect sprites
    GBC_Graphics_oam_set_sprite(graphics, 0, 0, 0, SPRITELET_GRASS, GBC_Graphics_attr_make(6, 2, false, false, false));
    GBC_Graphics_oam_set_sprite(graphics, 1, 0, 0, SPRITELET_GRASS, GBC_Graphics_attr_make(6, 2, true, false, false));
    GBC_Graphics_set_sprite_palette(graphics, 6, 0b11101101, 0b11011000, 0b11000100, 0b11000000);

    // Crate player sprites
    uint8_t base_sprite = s_player_sprites[2];
    uint8_t base_palette = s_switch_character ? 1 : 0;
    GBC_Graphics_oam_set_sprite(graphics, 2, s_player_x, s_player_y - 4, base_sprite, GBC_Graphics_attr_make(base_palette, 2, false, false, false));
    GBC_Graphics_oam_set_sprite(graphics, 3, s_player_x + TILE_WIDTH, s_player_y - 4, base_sprite+1, GBC_Graphics_attr_make(base_palette, 2, false, false, false));
    GBC_Graphics_oam_set_sprite(graphics, 4, s_player_x, s_player_y + TILE_HEIGHT - 4, base_sprite+2, GBC_Graphics_attr_make(base_palette, 2, false, false, false));
    GBC_Graphics_oam_set_sprite(graphics, 5, s_player_x + TILE_WIDTH, s_player_y + TILE_HEIGHT - 4, base_sprite+3, GBC_Graphics_attr_make(base_palette, 2, false, false, false));
    GBC_Graphics_set_sprite_palette(graphics, 0, 0b11111111, 0b11111001, 0b11110100, 0b11000000);
    GBC_Graphics_set_sprite_palette(graphics, 1, 0b11111111, 0b11111001, 0b11011011, 0b11000000);

    GBC_Graphics_lcdc_set_sprite_layer_enabled(graphics, true);
    GBC_Graphics_render(graphics);
}

void Pokemon_deinitialize(GBC_Graphics *graphics) {
  return;
}

// Yeah yeah I could make this work for more sprites (e.g. npcs) but this is a demo
static void set_player_sprites(GBC_Graphics *graphics, bool walk_sprite, bool x_flip) {
  const uint8_t new_tile = (s_switch_character ? kris_sprites : chris_sprites)[s_player_direction + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, 2+i, new_tile+i);
    GBC_Graphics_oam_set_sprite_x_flip(graphics, 2+i, x_flip);
  }

  if (x_flip) {
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 2, 3);
    GBC_Graphics_oam_swap_sprite_tiles(graphics, 4, 5);
  }
}

static void move_player_sprites(GBC_Graphics *graphics, short dx, short dy) {
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_move_sprite(graphics, 2+i, dx, dy);
  }
}

void Pokemon_step(GBC_Graphics *graphics) {
  // s_poll_frame = (s_poll_frame + 1) % 8;
  // AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  // Dir old_direction = s_player_direction;
  if (s_select_pressed && s_player_mode == P_STAND) {
    s_player_mode = P_WALK;
    s_walk_frame = 0;
    s_flip_walk = !s_flip_walk;
    GBC_Graphics_oam_set_sprite_priority(graphics, 4, false);
    GBC_Graphics_oam_set_sprite_priority(graphics, 5, false);
    
    s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
    s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
    
    s_can_move = get_square_info(s_target_x, s_target_y) != BLOCKING;

    if (!s_can_move) {
      s_target_x = s_player_x;
      s_target_y = s_player_y;
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
        move_player_sprites(graphics, direction_to_point(s_player_direction).x * 2, 
                            direction_to_point(s_player_direction).y * 2);
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
      }
      if (get_square_info(s_target_x, s_target_y)) {
        switch(s_walk_frame) {
          case 2:
          case 3:
          case 4:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_x, s_player_y + 4);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_x + 8, s_player_y + 4);
            break;
          case 5:
          case 6:
            GBC_Graphics_oam_set_sprite_pos(graphics, 0, s_player_x - 1, s_player_y + 5);
            GBC_Graphics_oam_set_sprite_pos(graphics, 1, s_player_x + 9, s_player_y + 5);
            break;
          case 7:
            GBC_Graphics_oam_hide_sprite(graphics, 0);
            GBC_Graphics_oam_hide_sprite(graphics, 1);
            break;
          default:
            break;
        }
      }
      switch(s_walk_frame) {
        case 7:
          s_player_mode = P_STAND;
          if(get_square_info(s_player_x, s_player_y) == GRASS) {
            GBC_Graphics_oam_set_sprite_priority(graphics, 4, true);
            GBC_Graphics_oam_set_sprite_priority(graphics, 5, true);
          }
        case 0:
        case 1:
        case 6:
          set_player_sprites(graphics, false,  s_player_direction == D_RIGHT);
          break;
        case 2:
        case 3:
        case 4:
        case 5:
          set_player_sprites(graphics, true,  s_player_direction == D_RIGHT 
                             || ((s_player_direction == D_DOWN || s_player_direction == D_UP) && s_flip_walk));
          break;
        default:
          break;
      }
      s_walk_frame++;
      break;
    default:
      break;
  }
}

void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed) {
  s_select_pressed = pressed;
}

void Pokemon_handle_down(GBC_Graphics *graphics) {
  if (s_player_mode == P_STAND) {
    s_player_direction = (s_player_direction + 1) % 4;
    set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
  }
}

void Pokemon_handle_tap(GBC_Graphics *graphics) {
  s_switch_character = !s_switch_character;
  s_player_sprites = s_switch_character ? kris_sprites : chris_sprites;
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_palette(graphics, 2+i, s_switch_character ? 1 : 0);
  }
  set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
}

void Pokemon_handle_focus_lost(GBC_Graphics *graphics) {

}