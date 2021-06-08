#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define NUM_MENU_ITEMS 5
#define MENU_ROOT_X 9
#define MENU_ROOT_Y 0
#define INFO_ROOT_X 2
#define INFO_ROOT_Y 13
#define PEBBLE_ROOT_X 1
#define PEBBLE_ROOT_Y 4
#define START_MENU_ROOT_X 0
#define START_MENU_ROOT_Y 1
#define START_INFO_ROOT_X 1
#define START_INFO_ROOT_Y 13
#define SAVE_KEY 1
#define DIALOGUE_BOUNDS GRect(0, 12, 18, 6)
#define DIALOGUE_ROOT GPoint(1, 14)
#define WILD_ODDS 10

#define BATTLE_MENU_BOUNDS GRect(10, 12, 8, 6)
#define BATTLE_SCROLL_SPEED 3
#define TILE_ENEMY_OFFSET 49
#define ENEMY_LOCATION GPoint(11, 0)
#define TILE_PLAYER_OFFSET 0
#define PLAYER_LOCATION GPoint(0, 5)

#define SAVE_MENU_BOUNDS GRect(0, 7, 6, 5)

#if defined(PBL_PLATFORM_DIORITE)
    #define FRAME_SKIP 8
#else
    #define FRAME_SKIP 1
#endif

#define PLAYER_ORIGIN_X 16*51
#define PLAYER_ORIGIN_Y 16*13

typedef struct {
    uint16_t player_x;
    uint16_t player_y;
    time_t last_save;
    uint16_t battles;
    uint16_t wins;
    uint16_t losses;
    uint16_t runs;
} PokemonSaveData;

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer);
void Pokemon_deinitialize(GBC_Graphics *graphics);
void Pokemon_step(GBC_Graphics *graphics);
void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed);
void Pokemon_handle_select_click(GBC_Graphics *graphics);
void Pokemon_handle_down(GBC_Graphics *graphics);
void Pokemon_handle_up(GBC_Graphics *graphics);
void Pokemon_handle_tap(GBC_Graphics *graphics);
void Pokemon_handle_back(GBC_Graphics *graphics);
void Pokemon_handle_focus_lost(GBC_Graphics *graphics);