#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define TILE_WATER_CAVE 56
#define TILE_WATER 130
#define TILE_FOUNTAIN 132
#define TILE_FLOWER 146
#define MAX_ANIM_FRAMES 48
#define NUM_ANIM_FRAMES 24
#define NUM_ANIMATIONS 4
#define WATER_PALETTE 3

void init_anim_tiles(GBC_Graphics *graphics, uint8_t anim_bank, uint8_t anim_offset);

void animate_tiles(GBC_Graphics *graphics, uint8_t tile_bank, uint8_t route);