#pragma once

#include <pebble.h>

// Sure yes this is non ideal, but this is a quick and dirty demo
#define SPRITE_CHRIS_STAND_UP 0
#define SPRITE_CHRIS_STAND_LEFT 4
#define SPRITE_CHRIS_STAND_DOWN 8
#define SPRITE_CHRIS_STAND_RIGHT 4
#define SPRITE_CHRIS_WALK_UP 12
#define SPRITE_CHRIS_WALK_LEFT 16
#define SPRITE_CHRIS_WALK_DOWN 20
#define SPRITE_CHRIS_WALK_RIGHT 16
#define SPRITE_KRIS_STAND_UP 24
#define SPRITE_KRIS_STAND_LEFT 28
#define SPRITE_KRIS_STAND_DOWN 32
#define SPRITE_KRIS_STAND_RIGHT 28
#define SPRITE_KRIS_WALK_UP 36
#define SPRITE_KRIS_WALK_LEFT 40
#define SPRITE_KRIS_WALK_DOWN 44
#define SPRITE_KRIS_WALK_RIGHT 40
#define SPRITELET_GRASS 48

const uint8_t chris_sprites[] = {
    SPRITE_CHRIS_STAND_UP,
    SPRITE_CHRIS_STAND_LEFT,
    SPRITE_CHRIS_STAND_DOWN,
    SPRITE_CHRIS_STAND_RIGHT,
    SPRITE_CHRIS_WALK_UP,
    SPRITE_CHRIS_WALK_LEFT,
    SPRITE_CHRIS_WALK_DOWN,
    SPRITE_CHRIS_WALK_RIGHT
};

const uint8_t kris_sprites[] = {
    SPRITE_KRIS_STAND_UP,
    SPRITE_KRIS_STAND_LEFT,
    SPRITE_KRIS_STAND_DOWN,
    SPRITE_KRIS_STAND_RIGHT,
    SPRITE_KRIS_WALK_UP,
    SPRITE_KRIS_WALK_LEFT,
    SPRITE_KRIS_WALK_DOWN,
    SPRITE_KRIS_WALK_RIGHT
};

typedef enum {
    P_DRT,
    P_GRS,
    P_TGR,
    P_C_W,
    P_CSW,
    P_C_S,
    P_CSE,
    P_C_E,
    P_SHR,
    P_T_T,
    P_T_B
} PokemonObject;

// These objects are *not* transposed!!
const uint8_t dirt[] = {
    2, 2,
    2, 2
};

const uint8_t grass[] = {
    1, 1,
    1, 1
};

const uint8_t tall_grass[] = {
    0, 0,
    0, 0
};

const uint8_t cliff_W[] = {
    3, 1,
    3, 1
};

const uint8_t cliff_SW[] = {
    3, 1,
    4, 5
};

const uint8_t cliff_S[] = {
    1, 1,
    5, 5
};

const uint8_t cliff_SE[] = {
    1, 7,
    5, 6
};

const uint8_t cliff_E[] = {
    1, 7,
    1, 7
};

const uint8_t shrub[] = {
    8, 9,
    12, 13
};

const uint8_t tree_top[] = {
    8, 9,
    10, 11
};

const uint8_t tree_bottom[] = {
    10, 11,
    12, 13
};

const uint8_t *Pokemon_object_array[] = {
    dirt,
    grass,
    tall_grass,
    cliff_W,
    cliff_SW,
    cliff_S,
    cliff_SE,
    cliff_E,
    shrub,
    tree_top,
    tree_bottom
};