#pragma once

#include <pebble.h>

typedef enum {
    PO_NONE, // data: 
    PO_SIGN, // data: dialogue 
    PO_TREE, // data: dialogue
    PO_WARP, // data: route warp, new x, new y
    PO_TRASH // data: dialogue
} PokemonObjectTypes;

// {route #, block x, block y, type, [data]}
// Route 1
const int16_t R01_SIGN_1[] = {0, 51, 11, PO_SIGN, 0};
const int16_t R01_SIGN_2[] = {0, 5, 9, PO_SIGN, 1};
const int16_t R01_TREE_1[] = {0, 21, 15, PO_TREE, 2};
const int16_t R01_TREE_2[] = {0, 30, 13, PO_TREE, 2};
const int16_t R01_DOOR_1[] = {0, 27, 5, PO_WARP, 1, 12*16, 47*16};

// Route 2
const int16_t R02_DOOR_1[] = {1, 12, 48, PO_WARP, 0, 27*16, 5*16};
const int16_t R02_DOOR_2[] = {1, 13, 48, PO_WARP, 0, 27*16, 5*16};
// TODO: Add in dialoge for the route 2 trash cans, signs

// I'm doing it like this so that I can create a jagged array
// This way, a dialogue object doesn't need to store 4 extra bytes of data
const int16_t *objects[] = {
    R01_SIGN_1,
    R01_SIGN_2,
    R01_TREE_1,
    R01_TREE_2,
    R01_DOOR_1,

    R02_DOOR_1,
    R02_DOOR_2
};

