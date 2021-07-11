#pragma once

#include <pebble.h>

typedef enum {
    PO_NONE, // data: 
    PO_TEXT, // data: dialogue index
    PO_TREE, // data: 
    PO_WARP, // data: route warp, new x, new y
} PokemonObjectTypes;

// {route #, block x, block y, type, [data]}
const int16_t CA_DOOR_1[] = {0, 19, 34, PO_WARP, 4, 6*16, 5*16};  // Cave
const int16_t CA_DOOR_2[] = {0, 19, 6, PO_WARP, 2, 16*16, 5*16};   

const int16_t FO_DOOR_1[] = {1, 5, 10, PO_WARP, 2, 35*16, 20*16}; // Forest
const int16_t FO_DOOR_2[] = {1, 5, 11, PO_WARP, 2, 35*16, 21*16};
const int16_t FO_DOOR_3[] = {1, 5, 46, PO_WARP, 3, 53*16, 12*16};
const int16_t FO_DOOR_4[] = {1, 5, 47, PO_WARP, 3, 53*16, 13*16};
const int16_t FO_SIGN_1[] = {1, 7, 21, PO_TEXT, 8};
const int16_t FO_SHRINE_1[] = {1, 12, 26, PO_TEXT, 9};
const int16_t FO_SHRINE_2[] = {1, 12, 27, PO_TEXT, 9};
const int16_t FO_TREE_1[] = {1, 12, 29, PO_TREE};

const int16_t NP_DOOR_1[] = {2, 11, 50, PO_WARP, 3, 27*16, 5*16}; // National Park
const int16_t NP_DOOR_2[] = {2, 12, 50, PO_WARP, 3, 27*16, 5*16};
const int16_t NP_DOOR_3[] = {2, 16, 5, PO_WARP, 0, 19*16, 5*16};
const int16_t NP_DOOR_4[] = {2, 36, 20, PO_WARP, 1, 6*16, 10*16};
const int16_t NP_DOOR_5[] = {2, 36, 21, PO_WARP, 1, 6*16, 11*16};
const int16_t NP_TRASH_1[] = {2, 10, 49, PO_TEXT, 10};
const int16_t NP_TRASH_2[] = {2, 13, 49, PO_TEXT, 10};
const int16_t NP_TRASH_3[] = {2, 14, 42, PO_TEXT, 10};
const int16_t NP_TRASH_4[] = {2, 32, 42, PO_TEXT, 10};
const int16_t NP_SIGN_1[] = {2, 16, 46, PO_TEXT, 11};
const int16_t NP_SIGN_2[] = {2, 29, 33, PO_TEXT, 12};
const int16_t NP_SIGN_3[] = {2, 14, 6, PO_TEXT, 13};

const int16_t R1_DOOR_1[] = {3, 27, 5, PO_WARP, 2, 11*16, 49*16}; // Route 1
const int16_t R1_DOOR_2[] = {3, 3, 10, PO_WARP, 4, 9*16, 60*16};
const int16_t R1_DOOR_3[] = {3, 3, 11, PO_WARP, 4, 9*16, 61*16};
const int16_t R1_DOOR_4[] = {3, 54, 12, PO_WARP, 1, 6*16, 46*16};
const int16_t R1_DOOR_5[] = {3, 54, 13, PO_WARP, 1, 6*16, 47*16};
const int16_t R1_SIGN_1[] = {3, 51, 11, PO_TEXT, 14};
const int16_t R1_SIGN_2[] = {3, 5, 9, PO_TEXT, 15};
const int16_t R1_TREE_1[] = {3, 21, 15, PO_TREE};
const int16_t R1_TREE_2[] = {3, 30, 13, PO_TREE};

const int16_t R2_DOOR_1[] = {4, 10, 60, PO_WARP, 3, 4*16, 10*16}; // Route 2
const int16_t R2_DOOR_2[] = {4, 10, 61, PO_WARP, 3, 4*16, 11*16};
const int16_t R2_DOOR_3[] = {4, 6, 5, PO_WARP, 0, 19*16, 33*16};
const int16_t R2_SIGN_1[] = {4, 9, 59, PO_TEXT, 16};
const int16_t R2_SIGN_2[] = {4, 7, 6, PO_TEXT, 17};

const int16_t *objects[] = {
    CA_DOOR_1,
    CA_DOOR_2,

    FO_DOOR_1,
    FO_DOOR_2,
    FO_DOOR_3,
    FO_DOOR_4,
    FO_SIGN_1,
    FO_SHRINE_1,
    FO_SHRINE_2,
    FO_TREE_1,

    NP_DOOR_1,
    NP_DOOR_2,
    NP_DOOR_3,
    NP_DOOR_4,
    NP_DOOR_5,
    NP_TRASH_1,
    NP_TRASH_2,
    NP_TRASH_3,
    NP_TRASH_4,
    NP_SIGN_1,
    NP_SIGN_2,
    NP_SIGN_3,

    R1_DOOR_1,
    R1_DOOR_2,
    R1_DOOR_3,
    R1_DOOR_4,
    R1_DOOR_5,
    R1_SIGN_1,
    R1_SIGN_2,
    R1_TREE_1,
    R1_TREE_2,

    R2_DOOR_1,
    R2_DOOR_2,
    R2_DOOR_3,
    R2_SIGN_1,
    R2_SIGN_2,
};

