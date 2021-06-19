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
const int16_t CA_DOOR_1[] = {0, 19, 34, PO_WARP, 4, 6*16, 5*16};  // Cave
const int16_t CA_DOOR_2[] = {0, 19, 6, PO_WARP, 2, 16*16, 5*16};   

const int16_t FO_DOOR_1[] = {1, 5, 10, PO_WARP, 2, 35*16, 20*16}; // Forest
const int16_t FO_DOOR_2[] = {1, 5, 11, PO_WARP, 2, 35*16, 21*16};
const int16_t FO_DOOR_3[] = {1, 5, 46, PO_WARP, 3, 53*16, 12*16};
const int16_t FO_DOOR_4[] = {1, 5, 47, PO_WARP, 3, 53*16, 13*16};

const int16_t NP_DOOR_1[] = {2, 11, 50, PO_WARP, 3, 27*16, 5*16}; // National Park
const int16_t NP_DOOR_2[] = {2, 12, 50, PO_WARP, 3, 27*16, 5*16};
const int16_t NP_DOOR_3[] = {2, 16, 5, PO_WARP, 0, 19*16, 5*16};
const int16_t NP_DOOR_4[] = {2, 36, 20, PO_WARP, 1, 6*16, 10*16};
const int16_t NP_DOOR_5[] = {2, 36, 21, PO_WARP, 1, 6*16, 11*16};

const int16_t R1_SIGN_1[] = {3, 51, 11, PO_SIGN, 0}; // Route 1
const int16_t R1_SIGN_2[] = {3, 5, 9, PO_SIGN, 1};
const int16_t R1_TREE_1[] = {3, 21, 15, PO_TREE, 2};
const int16_t R1_TREE_2[] = {3, 30, 13, PO_TREE, 2};
const int16_t R1_DOOR_1[] = {3, 27, 5, PO_WARP, 2, 11*16, 49*16};
const int16_t R1_DOOR_2[] = {3, 3, 10, PO_WARP, 4, 9*16, 60*16};
const int16_t R1_DOOR_3[] = {3, 3, 11, PO_WARP, 4, 9*16, 61*16};
const int16_t R1_DOOR_4[] = {3, 54, 12, PO_WARP, 1, 6*16, 46*16};
const int16_t R1_DOOR_5[] = {3, 54, 13, PO_WARP, 1, 6*16, 47*16};

const int16_t R2_DOOR_1[] = {4, 10, 60, PO_WARP, 3, 4*16, 10*16}; // Route 2
const int16_t R2_DOOR_2[] = {4, 10, 61, PO_WARP, 3, 4*16, 11*16};
const int16_t R2_DOOR_3[] = {4, 6, 5, PO_WARP, 0, 19*16, 33*16};

// TODO: Add in dialogue for the national park trash cans, signs

// I'm doing it like this so that I can create a jagged array
// This way, a dialogue object doesn't need to store 4 extra bytes of data
const int16_t *objects[] = {
    CA_DOOR_1,
    CA_DOOR_2,
    FO_DOOR_1,
    FO_DOOR_2,
    FO_DOOR_3,
    FO_DOOR_4,
    NP_DOOR_1,
    NP_DOOR_2,
    NP_DOOR_3,
    NP_DOOR_4,
    NP_DOOR_5,
    R1_SIGN_1,
    R1_SIGN_2,
    R1_TREE_1,
    R1_TREE_2,
    R1_DOOR_1,
    R1_DOOR_2,
    R1_DOOR_3,
    R1_DOOR_4,
    R1_DOOR_5,
    R2_DOOR_1,
    R2_DOOR_2,
    R2_DOOR_3,
};

