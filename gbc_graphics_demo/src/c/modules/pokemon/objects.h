#pragma once

#include <pebble.h>

typedef enum {
    PO_NONE,
    PO_SIGN,
    PO_CUT_TREE,
    PO_DOOR
} PokemonObjectTypes;

const int16_t pokemon_objects[][5] = {
    // {route, block x, block y, type, data}
    {0, 51, 11, PO_SIGN, 0}, // Sign #1 (data: dialogue)
    {0, 5, 9, PO_SIGN, 1}, // Sign #2 (data: dialogue)
    {0, 21, 15, PO_CUT_TREE, 2}, // Cut tree #1 (data: dialogue)
    {0, 30, 13, PO_CUT_TREE, 2}, // Cut tree #2 (data: dialogue)
    {0, 27, 5, PO_DOOR, 3}, // Door (data: dialogue) (in full example, could be warp data)
};

