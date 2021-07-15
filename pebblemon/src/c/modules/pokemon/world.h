#pragma once

#include <pebble.h>
#include "routes/Cave.h"
#include "routes/Forest.h"
#include "routes/NationalPark.h"
#include "routes/Route1.h"
#include "routes/Route2.h"
#include "overworld_palettes.h"

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

char *route_names[] = {
	"CAVE",
	"FOREST",
	"NATIONAL PARK",
	"ROUTE 1",
	"MOUNTAIN PASS",
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

uint8_t *overworld_morning_palettes[] = {
	cave_palettes,
	morning_palettes,
	morning_palettes,
	morning_palettes,
	morning_palettes,
};

uint8_t *overworld_day_palettes[] = {
	cave_palettes,
	day_palettes,
	day_palettes,
	day_palettes,
	day_palettes,
};

uint8_t *overworld_night_palettes[] = {
	cave_palettes,
	night_palettes,
	night_palettes,
	night_palettes,
	night_palettes,
};

uint8_t replacement_blocks[] = { // to replace the CUT trees
	0,
	18,
	6,
	16,
	23,
};