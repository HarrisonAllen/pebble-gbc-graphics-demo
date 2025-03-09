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
#define STATS_ROOT_X 0
#define STATS_ROOT_Y 0
#define OPTION_ROOT_X 0
#define OPTION_ROOT_Y 2
#define START_MENU_ROOT_X 0
#define START_MENU_ROOT_Y 1
#define START_INFO_ROOT_X 1
#define START_INFO_ROOT_Y 12

#define DIALOGUE_BOUNDS GRect(0, 12, 18, 6)
#define DIALOGUE_ROOT GPoint(1, 14)

#define WILD_ODDS 6
#define STEPS_BETWEEN_ENCOUNTERS 3
#define SAVE_KEY 1

#define BATTLE_MENU_BOUNDS GRect(10, 12, 8, 6)
#define BATTLE_SCROLL_SPEED 3
#define TILE_ENEMY_OFFSET 49
#define ENEMY_LOCATION GPoint(11, 0)
#define TILE_PLAYER_OFFSET 0
#define PLAYER_LOCATION GPoint(0, 5)

#define SAVE_MENU_BOUNDS GRect(0, 7, 6, 5)
#define CONFIRM_BOUNDS GRect(12, 7, 6, 5)

#if defined(PBL_PLATFORM_DIORITE)
    #define FRAME_SKIP 8
#else
    #define FRAME_SKIP 1
#endif

#define PLAYER_ORIGIN_X 16*10
#define PLAYER_ORIGIN_Y 16*10

#define TILE_BANK_MENU 0
#define TILE_BANK_WORLD 0
#define TILE_BANK_SPRITES 0
#define TILE_BANK_POKEMON 0

#define ENCOUNTERS_ENABLED true
#define DEMO_MODE false
#define RAND_SPRITES false

#define NUM_POKEMON 151

typedef struct {
    uint8_t player_level;
    int player_exp;
} PokemonSaveData;

void Pokemon_initialize(GBC_Graphics *graphics, Layer *background_layer);
void Pokemon_deinitialize(GBC_Graphics *graphics);
void Pokemon_step(GBC_Graphics *graphics);
void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed);
void Pokemon_handle_select_click(GBC_Graphics *graphics);
void Pokemon_handle_down(GBC_Graphics *graphics);
void Pokemon_handle_up(GBC_Graphics *graphics);
void Pokemon_handle_back(GBC_Graphics *graphics);
void Pokemon_handle_focus_lost(GBC_Graphics *graphics);