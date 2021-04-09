#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#if defined(PBL_COLOR)
#define FPS_DELAY 1
#else
#define FPS_DELAY 8
#endif
#define POKEMON_NUM_MENU_ITEMS 6
#define POKEMON_MENU_ROOT_X 9
#define POKEMON_MENU_ROOT_Y 0
#define POKEMON_INFO_ROOT_X 2
#define POKEMON_INFO_ROOT_Y 14

typedef struct {
    // TODO: Want to look into:
    //  - Checking if save data exists (what does api return if no data at key?)

    uint16_t player_x;
    uint8_t player_y;
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