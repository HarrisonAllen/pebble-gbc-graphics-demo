#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

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
#define POKEMON_DIALOGUE_BOUNDS GRect(0, 12, 18, 6)
#define POKEMON_DIALOGUE_ROOT GPoint(1, 14)
#define POKEMON_WILD_ODDS 2

#define POKEMON_BATTLE_MENU_BOUNDS GRect(10, 12, 8, 6)
#define POKEMON_BATTLE_SCROLL_SPEED 3
#define POKEMON_TILE_ENEMY_OFFSET 49
#define POKEMON_ENEMY_LOCATION GPoint(11, 0)
#define POKEMON_TILE_PLAYER_OFFSET 0
#define POKEMON_PLAYER_LOCATION GPoint(0, 5)

#define POKEMON_SAVE_MENU_BOUNDS GRect(0, 7, 6, 5)

#if defined(PBL_PLATFORM_DIORITE)
    #define POKEMON_FRAME_SKIP 8
#else
    #define POKEMON_FRAME_SKIP 1
#endif

#define POKEMON_PLAYER_ORIGIN_X 16*51
#define POKEMON_PLAYER_ORIGIN_Y 16*13

typedef struct {
    uint16_t player_x;
    uint16_t player_y;
    time_t last_save;
    uint16_t battles;
    uint16_t wins;
    uint16_t losses;
    uint16_t runs;
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