#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#if defined(PBL_COLOR)
#define FPS_DELAY 1
#else
#define FPS_DELAY 8
#endif
#define POKEMON_NUM_MENU_ITEMS 5
#define POKEMON_MENU_ROOT_X 9
#define POKEMON_MENU_ROOT_Y 0
#define POKEMON_INFO_ROOT_X 2
#define POKEMON_INFO_ROOT_Y 13
#define POKEMON_PEBBLE_ROOT_X 1
#define POKEMON_PEBBLE_ROOT_Y 4
#define POKEMON_START_MENU_ROOT_X 0
#define POKEMON_START_MENU_ROOT_Y 1
#define POKEMON_START_INFO_ROOT_X 1
#define POKEMON_START_INFO_ROOT_Y 13
#define POKEMON_SAVE_KEY 1

#define POKEMON_PLAYER_ORIGIN_X 16*51
#define POKEMON_PLAYER_ORIGIN_Y 16*13

typedef struct {
    uint16_t player_x;
    uint16_t player_y;
    time_t last_save;
} PokemonSaveData;

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer, void (*next_demo_callback)());
void Pokemon_deinitialize(GBC_Graphics *graphics);
void Pokemon_step(GBC_Graphics *graphics);
void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed);
void Pokemon_handle_select_click(GBC_Graphics *graphics);
void Pokemon_handle_down(GBC_Graphics *graphics);
void Pokemon_handle_up(GBC_Graphics *graphics);
void Pokemon_handle_tap(GBC_Graphics *graphics);
void Pokemon_handle_back(GBC_Graphics *graphics);
void Pokemon_handle_focus_lost(GBC_Graphics *graphics);