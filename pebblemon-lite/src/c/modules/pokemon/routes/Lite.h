#pragma once

#include <pebble.h>

uint8_t lite_chunks[] = {
    0, 0, 1, 1, // Tree
    2, 2, 2, 2, // Short grass
    3, 3, 3, 3, // Empty
    4, 4, 4, 4, // Tall grass
};

uint8_t lite_blocks[] = {
	1, 3, 2, 4,
	2, 4, 5, 6,
	9, 9, 9, 9,
	8, 8, 8, 8,
	7, 7, 7, 7,
};

uint8_t lite_block_types[] = {
	0,
	0,
	1,
	1,
	2,
};

uint8_t lite_tile_palettes[] = {
	0,
	0,
	2,
	5,
	5,
};