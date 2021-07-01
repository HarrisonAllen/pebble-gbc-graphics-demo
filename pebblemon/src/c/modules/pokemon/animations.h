#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define TILE_WATER_CAVE 56
#define TILE_WATER 130
#define TILE_FOUNTAIN 132
#define TILE_FLOWER 146
#define NUM_ANIM_FRAMES 24
#define NUM_ANIMATIONS 4

void init_anim_tiles(GBC_Graphics *graphics, uint8_t anim_bank, uint8_t anim_offset);

void animate_tiles(GBC_Graphics *graphics, uint8_t tile_bank);