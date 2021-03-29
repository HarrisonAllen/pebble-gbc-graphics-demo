#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define TEXT_START 58
#define NUMBER_START 84
#define SYMBOL_START 94
#define MAX_WALK_SPEED 2
#define MAX_RUN_SPEED 5
#define MAX_JUMP_SPEED 5
#define MAX_FALL_SPEED -100
#define MAX_JUMP_HEIGHT 64
#define MIN_JUMP_HEIGHT 16
#define WORLD_LENGTH 213 // Columns
#define WORLD_HEIGHT 11 // Rows
#define MD_NONE 0b0000
#define MD_UP 0b0001
#define MD_LEFT 0b0010
#define MD_DOWN 0b0100
#define MD_RIGHT 0b1000
#define NUM_MENU_ITEMS 6

typedef enum {
    MG_PLAY,
    MG_PAUSE_MENU_MOVING,
    MG_PAUSE,
    MG_PLAY_MENU_MOVING
} MarioGameState;

typedef enum {
    MJ_STANDING,
    MJ_JUMPING,
    MJ_FALLING
} MarioJumpState;

typedef struct {
    // TODO: Add values to save, e.g. player x, player y, coins, time, etc etc
    // Then store them with the Storage API https://developer.rebble.io/developer.pebble.com/guides/events-and-services/persistent-storage/index.html
    // Want to look into:
    //  - Checking if save data exists (what does api return if no data at key?)
} MarioSaveData;

void Mario_initialize(GBC_Graphics *graphics);
void Mario_deinitialize(GBC_Graphics *graphics);
void Mario_step(GBC_Graphics *graphics);
void Mario_load_column_at_pos(GBC_Graphics *graphics, uint16_t column, uint8_t bg_tile_x, uint8_t bg_tile_y);
void Mario_handle_select(GBC_Graphics *graphics);
void Mario_handle_back(GBC_Graphics *graphics);
void Mario_handle_down(GBC_Graphics *graphics, bool down);
void Mario_handle_up(GBC_Graphics *graphics, bool up);
void Mario_handle_down_click(GBC_Graphics *graphics);
void Mario_handle_up_click(GBC_Graphics *graphics);
void Mario_handle_tap(GBC_Graphics *graphics);
void Mario_handle_focus_lost(GBC_Graphics *graphics);