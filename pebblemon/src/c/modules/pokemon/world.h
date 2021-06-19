#pragma once

#include <pebble.h>
#include "routes/Cave.h"
#include "routes/Forest.h"
#include "routes/NationalPark.h"
#include "routes/Route1.h"
#include "routes/Route2.h"

uint8_t route_dims[] = {
//  width, height
	12, 20, // Cave
	19, 26, // Forest
	21, 26, // National Park
	29, 13, // Route 1
	7, 33, // Route 2
};

uint32_t map_files[] = {
	RESOURCE_ID_DATA_CAVE,
	RESOURCE_ID_DATA_FOREST,
	RESOURCE_ID_DATA_NATIONAL_PARK,
	RESOURCE_ID_DATA_ROUTE_1,
	RESOURCE_ID_DATA_ROUTE_2,
};

typedef enum {
    BLOCK,
    WALK,
    GRASS,
    OBJECT,
	CLIFF_S, // Jump from North
	CLIFF_W, // Jump from East
	CLIFF_E, // Jump from West
	WATER
} PokemonSquareInfo;

uint8_t *chunks[] = {
	cave_chunks,
	forest_chunks,
	national_park_chunks,
	route_1_chunks,
	route_2_chunks,
};

uint8_t *blocks[] = {
	cave_blocks,
	forest_blocks,
	national_park_blocks,
	route_1_blocks,
	route_2_blocks,
};

uint8_t *block_types[] = {
	cave_block_types,
	forest_block_types,
	national_park_block_types,
	route_1_block_types,
	route_2_block_types,
};

uint8_t *tile_palettes[] = {
	cave_tile_palettes,
	forest_tile_palettes,
	national_park_tile_palettes,
	route_1_tile_palettes,
	route_2_tile_palettes,
};

uint8_t *palettes[] = {
	cave_palettes,
	forest_palettes,
	national_park_palettes,
	route_1_palettes,
	route_2_palettes,
};
