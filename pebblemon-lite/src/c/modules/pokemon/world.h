#pragma once

#include <pebble.h>
#include "routes/Lite.h"
#include "overworld_palettes.h"

uint8_t route_dims[] = {
//  width, height
	10, 10 // Lite
};

uint32_t map_files[] = {
	RESOURCE_ID_DATA_LITE
};

char *route_names[] = {
	"LITE"
};

typedef enum {
    BLOCK,
    WALK,
    GRASS,
    OBJECT,
	CLIFF_S, // Jump from North
	CLIFF_W, // Jump from East
	CLIFF_E, // Jump from West
	WATER,
	CLIFF_N, // No jump, can't walk from North
} PokemonSquareInfo;

uint8_t *chunks[] = {
	lite_chunks
};

uint8_t *blocks[] = {
	lite_blocks
};

uint8_t *block_types[] = {
	lite_block_types
};


uint8_t *tile_palettes[] = {
	lite_tile_palettes,
};

uint8_t *overworld_palettes[] = {
	palettes,
};