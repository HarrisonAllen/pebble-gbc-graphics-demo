#include "pokemon.h"
#include "menu.h"
#include "world.h"
#include "sprites.h"
#include "sprite_decompressor/decompressor.h"
#include "enums.h"

// Player variables
static PlayerDirection s_player_direction = D_DOWN;
static PlayerMode s_player_mode;
static uint16_t s_player_x, s_player_y;
static uint8_t s_player_sprite_x, s_player_sprite_y;
static uint8_t *s_player_palette;
static uint8_t s_player_level = DEMO_MODE ? 100 : 1;
static uint8_t s_route_num = 0;

// Player stats
static uint16_t s_stats_battles, s_stats_wins, s_stats_losses;
static int s_player_exp;
static int s_to_next_level, s_to_cur_level;
static int s_exp_gained = DEMO_MODE ? 100*100*100 : 0;

// States
static PokemonGameState s_game_state, s_prev_game_state;
static PokemonBattleState s_battle_state;

// Battle
static uint8_t s_player_pokemon_name[11], s_enemy_pokemon_name[11];
static uint16_t s_player_pokemon, s_enemy_pokemon;
static uint8_t s_enemy_pokemon_level;
static uint16_t s_player_max_pokemon_health, s_enemy_max_pokemon_health;
static uint16_t s_player_pokemon_health, s_enemy_pokemon_health;
static uint8_t s_player_pokemon_damage, s_enemy_pokemon_damage;
static uint16_t s_player_pokemon_attack, s_enemy_pokemon_attack;
static uint16_t s_player_pokemon_defense, s_enemy_pokemon_defense;
static uint8_t s_escape_odds;
static bool s_player_goes_first;

// Frame counters
static uint8_t s_walk_frame, s_poll_frame, s_battle_frame, 
               s_step_frame, s_demo_frame;

// Movement
static bool s_flip_walk, s_can_move;
static uint16_t s_target_x, s_target_y;
static bool s_up_press_queued, s_down_press_queued;
static uint16_t s_steps_since_last_encounter;

// Options
static bool s_save_file_exists;
static bool s_move_toggle;
static uint8_t s_player_sprite = 0;
static uint8_t s_text_speed = 2;
int s_accel_x_cal, s_accel_y_cal;

// Misc
static bool s_select_pressed;
static uint8_t *s_world_map;
static bool s_clear_dialogue = true;
static uint8_t s_cur_bg_palettes[PALETTE_BANK_SIZE];
static bool s_game_started;

// Tilebank offsets
static uint16_t s_bg_tile_offset, s_ui_tile_offset, s_sprite_tile_offset, s_pkmn_tile_offset;

static int cube(int x) {
  return x*x*x;
}

static GPoint direction_to_point(PlayerDirection dir) {
    switch (dir) {
        case D_UP:  return GPoint(0, -1);
        case D_LEFT:  return GPoint(-1, 0);
        case D_DOWN:  return GPoint(0, 1);
        case D_RIGHT:  return GPoint(1, 0);
        default: return GPoint(0, 0);
    }
}

static uint8_t get_tile_id_from_map(uint8_t *map, uint16_t tile_x, uint16_t tile_y) {
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return blocks[s_route_num][block * 4 + (tile_x & 1) + (tile_y & 1) * 2];
}

static void load_tiles(GBC_Graphics *graphics, int16_t bg_root_x, int16_t bg_root_y, uint16_t tile_root_x, uint16_t tile_root_y, uint8_t num_x_tiles, uint8_t num_y_tiles) {
  bg_root_x = (bg_root_x + MAP_WIDTH) % (MAP_WIDTH);
  bg_root_y = (bg_root_y + MAP_HEIGHT) % (MAP_HEIGHT);
  for (uint16_t tile_y = tile_root_y; tile_y < tile_root_y + num_y_tiles; tile_y++) {
    for (uint16_t tile_x = tile_root_x; tile_x < tile_root_x + num_x_tiles; tile_x++) {
      uint8_t tile = get_tile_id_from_map(s_world_map, tile_x, tile_y);
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bg_root_x + (tile_x - tile_root_x), bg_root_y + (tile_y - tile_root_y), tile + s_bg_tile_offset, 
                                          GBC_Graphics_attr_make(tile_palettes[s_route_num][tile], TILE_BANK_WORLD, false, false, false));
    }
  }
}

static void set_bg_palettes(GBC_Graphics *graphics) {
  uint8_t *palette;
  palette = overworld_palettes[0];
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_set_bg_palette_array(graphics, i, &palette[i*PALETTE_SIZE]);
  }
}

static void load_screen_no_reset(GBC_Graphics *graphics) {
  GBC_Graphics_bg_set_scroll_pos(graphics, s_player_x % 8, s_player_y % 8);
  int16_t bg_root_x = GBC_Graphics_bg_get_scroll_x(graphics) >> 3;
  int16_t bg_root_y = GBC_Graphics_bg_get_scroll_y(graphics) >> 3;
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  load_tiles(graphics, bg_root_x, bg_root_y, tile_root_x, tile_root_y, 19, 19);
}

static void load_screen(GBC_Graphics *graphics) {
  set_bg_palettes(graphics);
  GBC_Graphics_copy_all_bg_palettes(graphics, s_cur_bg_palettes);
  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);
  int16_t bg_root_x = GBC_Graphics_bg_get_scroll_x(graphics) >> 3;
  int16_t bg_root_y = GBC_Graphics_bg_get_scroll_y(graphics) >> 3;
  uint16_t tile_root_x = (s_player_x >> 3) - 8;
  uint16_t tile_root_y = (s_player_y >> 3) - 8;
  load_tiles(graphics, bg_root_x, bg_root_y, tile_root_x, tile_root_y, 19, 19);
}

static void reset_player_palette(GBC_Graphics *graphics) {
  s_player_palette = pokemon_trainer_palettes[s_player_sprite];
  GBC_Graphics_set_sprite_palette(graphics, 0, s_player_palette[0], s_player_palette[1], s_player_palette[2], s_player_palette[3]);
}

static void set_player_sprites(GBC_Graphics *graphics, bool walk_sprite, bool x_flip) {
  uint8_t new_tile = pokemon_trainer_sprite_offsets[s_player_direction + walk_sprite * 4];
  for (uint8_t i = 0; i < 4; i++) {
    GBC_Graphics_oam_set_sprite_tile(graphics, 2 + i, (new_tile * POKEMON_TILES_PER_SPRITE) + i + s_sprite_tile_offset);
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
  uint8_t chunk = map[(tile_x >> 2) + (tile_y >> 2) * route_dims[s_route_num << 1]];
  uint8_t block = chunks[s_route_num][chunk * 4 + ((tile_x >> 1) & 1) + ((tile_y >> 1) & 1) * 2];
  return block_types[s_route_num][block];
}

static void load_overworld(GBC_Graphics *graphics) {
  load_screen(graphics);
  
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  // Create player sprites
  for (uint8_t y = 0; y < 2; y++) {
    for (uint8_t x = 0; x < 2; x++) {
      GBC_Graphics_oam_set_sprite_pos(graphics, 2 + x + 2 * y, s_player_sprite_x + x * 8, s_player_sprite_y + y * 8 - 4);
      GBC_Graphics_oam_set_sprite_attrs(graphics, 2 + x + 2 * y, GBC_Graphics_attr_make(0, TILE_BANK_SPRITES, false, false, false));
    }
  }
  set_player_sprites(graphics, false, s_player_direction == D_RIGHT);
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
      GBC_Graphics_bg_set_tile_and_attrs(graphics, location.x + x, location.y + y, i+vram_tile_offset, GBC_Graphics_attr_make(palette, TILE_BANK_POKEMON, false, false, false));
      i++;
    }
  }
}

static void load_palette(GBC_Graphics *graphics, uint16_t pokemon_number, uint8_t palette) {
  GBC_Graphics_set_bg_palette(graphics, palette, 1, 1, 0, 0);
  GBC_Graphics_set_sprite_palette(graphics, palette, 1, 1, 0, 0);
}

static void load_pokemon(GBC_Graphics *graphics, uint16_t pokemon_number, bool front, GPoint location, uint8_t vram_tile_offset, uint8_t palette) {
  uint8_t *pokemon_bank = GBC_Graphics_get_vram_bank(graphics, TILE_BANK_POKEMON) + vram_tile_offset * 16;
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

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer) {
  s_game_state = PG_PLAY;
  GBC_Graphics_set_screen_bounds(graphics, SCREEN_BOUNDS_SQUARE);
  GBC_Graphics_window_set_offset_pos(graphics, 0, 168);
  GBC_Graphics_lcdc_set_8x16_sprite_mode_enabled(graphics, false);
  for (uint8_t i = 0; i < 40; i++) {
      GBC_Graphics_oam_hide_sprite(graphics, i);
  }

  set_bg_palettes(graphics);

  s_ui_tile_offset = 0;
  ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_MENU_TILESHEET);
  size_t res_size = resource_size(handle);
  uint16_t tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_MENU_TILESHEET, 
                                             0, tiles_to_load, s_ui_tile_offset, TILE_BANK_MENU);

  s_bg_tile_offset = s_ui_tile_offset + tiles_to_load;
  handle = resource_get_handle(RESOURCE_ID_DATA_WORLD_TILESHEET);
  res_size = resource_size(handle);
  tiles_to_load = res_size / 16;
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_WORLD_TILESHEET,
                                             0, tiles_to_load, s_bg_tile_offset, TILE_BANK_WORLD);

  s_sprite_tile_offset = s_bg_tile_offset + tiles_to_load;
  handle = resource_get_handle(RESOURCE_ID_DATA_SPRITESHEET);
  res_size = resource_size(handle);
  tiles_to_load = res_size / 16 - 2;
  reset_player_palette(graphics);
  GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_SPRITESHEET,
                                             0, tiles_to_load, s_sprite_tile_offset, TILE_BANK_SPRITES); // offset 10 for effects and items

  s_pkmn_tile_offset = s_sprite_tile_offset + tiles_to_load;

  GBC_Graphics_stat_set_line_compare_interrupt_enabled(graphics, false);
  GBC_Graphics_lcdc_set_bg_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, true);
  GBC_Graphics_lcdc_set_sprite_layer_enabled(graphics, true);

  GBC_Graphics_bg_set_scroll_pos(graphics, 0, 0);

  PokemonSaveData data;
  if (load(&data)) {
    s_player_level = data.player_level;
    s_to_next_level = cube(s_player_level+1);
    s_to_cur_level = s_player_level == 1 ? 0 : cube(s_player_level);
    s_player_exp = data.player_exp;
  } else {
    s_to_next_level = cube(s_player_level+1);
    s_to_cur_level = s_player_level == 1 ? 0 : cube(s_player_level);
  }
  s_player_x = PLAYER_ORIGIN_X;
  s_player_y = PLAYER_ORIGIN_Y;
  load_game(graphics);
  s_game_state = PG_PLAY;
  s_select_pressed = false;
  s_game_started = true;
}

static void play(GBC_Graphics *graphics) {
  s_demo_frame = (s_demo_frame + 1) % 16;
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
    if (s_move_toggle) {
      s_target_x = s_player_x + direction_to_point(s_player_direction).x * (TILE_WIDTH * 2);
      s_target_y = s_player_y + direction_to_point(s_player_direction).y * (TILE_HEIGHT * 2);
      
      s_player_mode = P_WALK;
      s_walk_frame = 0;
      if (s_player_sprite != 21) {
        s_flip_walk = !s_flip_walk;
      }
      PokemonSquareInfo target_block_type = get_block_type(s_world_map, s_target_x+8, s_target_y);
      s_can_move = (target_block_type == WALK || target_block_type == GRASS);
      if (!s_can_move) {
        s_target_x = s_player_x;
        s_target_y = s_player_y;
      } else {
        GBC_Graphics_oam_hide_sprite(graphics, 0);
        GBC_Graphics_oam_hide_sprite(graphics, 1);
        s_steps_since_last_encounter++;
      }
    }
  }
  
  switch(s_player_mode) {
    case P_STAND:
      break;
    case P_WALK:
      if (s_can_move) {
        s_player_x += direction_to_point(s_player_direction).x * 2;
        s_player_y += direction_to_point(s_player_direction).y * 2;
        GBC_Graphics_bg_move(graphics, direction_to_point(s_player_direction).x * 2, direction_to_point(s_player_direction).y * 2);
      }
      if (s_walk_frame == 7 && s_can_move && get_block_type(s_world_map, s_target_x+8, s_target_y) == GRASS) {
        if (ENCOUNTERS_ENABLED && (rand() % WILD_ODDS == 0) && (s_steps_since_last_encounter >= STEPS_BETWEEN_ENCOUNTERS)) {
          s_game_state = PG_BATTLE;
          s_battle_state = PB_LOAD;
          s_battle_frame = 0;
          s_move_toggle = false;
          s_steps_since_last_encounter = 0;
        }
      }
      switch(s_walk_frame) {
        case 7:
          s_player_mode = P_STAND;
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
    default:
      break;
  }
  load_screen_no_reset(graphics);
}


static uint16_t calculate_health(uint8_t level) {
  uint8_t base = rand()%100 + 50;
  return ((base + 50) * level) / 50 + 10;
}

static uint16_t calculate_damage(uint8_t level, uint16_t attack, uint16_t defense) {
  uint16_t crit = (rand()%20==0) ? 1.5 : 1;
  return (uint16_t)(((((2.0 * level / 5.0 + 2.0) * attack * 80.0 / defense) / 50) + 2) * (rand()%16 + 85) / 100) * crit;
}

static uint16_t calculate_exp_gain(uint8_t enemy_level) {
  uint16_t effort_value = rand()%200 + 100;
  return effort_value * enemy_level  / 6;
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
  s_player_pokemon_attack = (rand()%15+5)*(s_player_level/10.0+1);
  s_enemy_pokemon_attack = (rand()%15+5)*(s_player_level/10.0+1);
  s_player_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1);
  s_enemy_pokemon_defense = (rand()%15+5)*(s_enemy_pokemon_level/10.0+1);
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
    case PB_LOAD:
      for (uint8_t i = 0; i < 40; i++) {
        GBC_Graphics_oam_hide_sprite(graphics, i);
      }
      // Load in the pokemon
      s_battle_frame = 0;
      s_battle_state = PB_APPEAR;
      for (uint8_t x = 0; x < 24; x++) {
        for (uint8_t y = 0; y < 24; y++) {
          GBC_Graphics_bg_set_tile_and_attrs(graphics, x, y, 0, GBC_Graphics_attr_make(0, TILE_BANK_MENU, false, false, false));
        }
      }
      s_player_pokemon = rand() % NUM_POKEMON + 1;
      s_enemy_pokemon = rand() % NUM_POKEMON + 1;
      load_pokemon(graphics, s_player_pokemon, false, PLAYER_LOCATION, s_pkmn_tile_offset + TILE_PLAYER_OFFSET, 1);
      load_pokemon(graphics, s_enemy_pokemon, true, ENEMY_LOCATION, s_pkmn_tile_offset + TILE_ENEMY_OFFSET, 2);
      ResHandle data_handle = resource_get_handle(RESOURCE_ID_DATA_POKEMON_NAMES);
      uint16_t data_offset = 10 * s_player_pokemon;
      resource_load_byte_range(data_handle, data_offset, s_player_pokemon_name, 10);
      data_offset = 10 * s_enemy_pokemon;
      resource_load_byte_range(data_handle, data_offset, s_enemy_pokemon_name, 10);

      draw_menu_rectangle(graphics, DIALOGUE_BOUNDS);

      generate_pokemon_stats();

      s_escape_odds = rand() % 32;
      load_palette(graphics, s_player_pokemon, 1);
      load_palette(graphics, s_enemy_pokemon, 2);
      break;
    case PB_SLIDE:
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
      draw_uint_string_at_location(graphics, GPoint(1, 0), s_enemy_pokemon_name, 11);
      draw_text_at_location(graphics, GPoint(4, 1), level_text);
      draw_uint_string_at_location(graphics, GPoint(8, 7), s_player_pokemon_name, 11);
      if (s_player_level < 100) {
        snprintf(level_text, 8, "|%-2d%c", s_player_level, rand()%2 ? '^' : '+');
      } else {
        snprintf(level_text, 8, "%d%c", s_player_level, rand()%2 ? '^' : '+');
      }
      draw_text_at_location(graphics, GPoint(12, 8), level_text);
      s_player_goes_first = rand() % 2;
    } case PB_PLAYER_TURN_PROMPT: {
      char attack_dialogue[40];
      snprintf(attack_dialogue, 40, "Press SELECT\nto attack!");
      begin_dialogue_from_string(graphics, DIALOGUE_BOUNDS, DIALOGUE_ROOT, attack_dialogue, true);
      s_prev_game_state = PG_BATTLE;
      s_game_state = PG_DIALOGUE;
      s_battle_state = PB_PLAYER_TURN;
    } break;
    case PB_PLAYER_TURN:
      if (s_player_goes_first) {
        s_battle_state = PB_PLAYER_MOVE;
      } else {
        s_battle_state = PB_ENEMY_MOVE;
      }
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
          GBC_Graphics_set_bg_palette(graphics, 7, 1, 1, 1, 1);
        }
        if (s_battle_frame % 4 == 2) {
          render_pokemon(graphics, ENEMY_LOCATION, s_pkmn_tile_offset + TILE_ENEMY_OFFSET, 7);
        } else if (s_battle_frame % 4 == 0) {
          render_pokemon(graphics, ENEMY_LOCATION, s_pkmn_tile_offset + TILE_ENEMY_OFFSET, 2);
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
          if (s_player_goes_first) {
            s_battle_state = PB_ENEMY_MOVE;
            s_battle_frame = 0;
          } else {
            s_battle_state = PB_PLAYER_TURN_PROMPT;
            s_battle_frame = 0;
          }
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
          GBC_Graphics_set_bg_palette(graphics, 7, 1, 1, 1, 1);
        }
        if (s_battle_frame % 4 == 2) {
          render_pokemon(graphics, PLAYER_LOCATION, s_pkmn_tile_offset + TILE_PLAYER_OFFSET, 7);
        } else if (s_battle_frame % 4 == 0) {
          render_pokemon(graphics, PLAYER_LOCATION, s_pkmn_tile_offset + TILE_PLAYER_OFFSET, 1);
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
          if (s_player_goes_first) {
            s_battle_state = PB_PLAYER_TURN_PROMPT;
            s_battle_frame = 0;
          } else {
            s_battle_state = PB_PLAYER_MOVE;
            s_battle_frame = 0;
          }
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
          GBC_Graphics_bg_set_tile(graphics, 11+x, 0, s_bg_tile_offset);
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
        s_exp_gained = calculate_exp_gain(s_enemy_pokemon_level);
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
            char exp_dialogue[120];
            snprintf(exp_dialogue, 120, "Lvl grew to %d!", s_player_level);
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
          GBC_Graphics_bg_set_tile(graphics, 0+x, 5, s_bg_tile_offset);
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
    case PB_FADEOUT:
      load_overworld(graphics);
      s_game_state = PG_PLAY;
      break;
  }
}

static void save() {
  PokemonSaveData data = (PokemonSaveData) {
    .player_level = s_player_level,
    .player_exp = s_player_exp,
  };
  persist_write_data(SAVE_KEY, &data, sizeof(PokemonSaveData));
  s_save_file_exists = true;
}

void Pokemon_step(GBC_Graphics *graphics) {
  s_step_frame = (s_step_frame + 1) % FRAME_SKIP;
  if (s_step_frame != 0) {
      return;
  }
  switch (s_game_state) {
    case PG_PLAY:
      play(graphics);
      break;
    case PG_DIALOGUE:
      if (get_dialogue_state() != D_IDLE) {
        step_dialogue(graphics, s_select_pressed, s_text_speed);
      } else {
        if (s_prev_game_state == PG_BATTLE) {
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

void Pokemon_handle_select_click(GBC_Graphics *graphics) {
  switch(s_game_state) {
    case PG_DIALOGUE:
      handle_input_dialogue(graphics);
      break;
    case PG_PLAY:
      s_move_toggle = !s_move_toggle;
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
    default:
      break;
  }
}

void Pokemon_handle_up(GBC_Graphics *graphics) {
  switch (s_game_state) {
    case PG_PLAY:
      s_up_press_queued = true;
      break;
    default:
      break;
  }
}

void Pokemon_handle_focus_lost(GBC_Graphics *graphics) {
  s_select_pressed = false;
}

void Pokemon_handle_back(GBC_Graphics *graphics) {
  window_stack_pop(true);
}

void Pokemon_deinitialize(GBC_Graphics *graphics) {
  if (s_game_started) {
    save();
  }
  unload_dialogue();
  if (s_world_map != NULL) {
    free(s_world_map);
    s_world_map = NULL;
  }
}