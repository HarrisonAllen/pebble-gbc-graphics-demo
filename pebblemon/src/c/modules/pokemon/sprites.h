#pragma once

#include <pebble.h>

#define POKEMON_TILES_PER_SPRITE 4
#define POKEMON_SPRITES_PER_TRAINER 6
#define POKEMON_TRAINER_STAND_DOWN 0
#define POKEMON_TRAINER_STAND_UP 1
#define POKEMON_TRAINER_STAND_LEFT 2
#define POKEMON_TRAINER_STAND_RIGHT 2
#define POKEMON_TRAINER_WALK_DOWN 3
#define POKEMON_TRAINER_WALK_UP 4
#define POKEMON_TRAINER_WALK_LEFT 5
#define POKEMON_TRAINER_WALK_RIGHT 5

// There are like a million more trainer sprites in the file, but this will do for demo
#define POKEMON_SPRITE_CHRIS 0
#define POKEMON_SPRITE_CHRIS_BIKE 6
#define POKEMON_SPRITE_KRIS 12
#define POKEMON_SPRITE_KRIS_BIKE 18
#define POKEMON_SPRITE_RIVAL 31
#define POKEMON_SPRITE_OAK 37
#define POKEMON_SPRITE_RED 43
#define POKEMON_SPRITE_BLUE 49

#define POKEMON_SPRITE_PARTY 428 // Yeah I'm not using them, but they're there!
#define POKEMON_SPRITE_EMOTICONS 504 // Okay not using these either, sue me
#define POKEMON_SPRITE_OVERWORLD_POKEMON 512 // Sure fine I know I'm wasting storage space
#define POKEMON_SPRITE_ITEMS 563
#define POKEMON_SPRITE_CUT_TREE 567
#define POKEMON_SPRITE_ITEM 566
#define POKEMON_SPRITELET_GRASS 578 // Only one tile!

const uint8_t pokemon_trainer_sprite_offsets[] = {
    POKEMON_TRAINER_STAND_UP,
    POKEMON_TRAINER_STAND_LEFT,
    POKEMON_TRAINER_STAND_DOWN,
    POKEMON_TRAINER_STAND_RIGHT,
    POKEMON_TRAINER_WALK_UP,
    POKEMON_TRAINER_WALK_LEFT,
    POKEMON_TRAINER_WALK_DOWN,
    POKEMON_TRAINER_WALK_RIGHT,
};

uint8_t pokemon_trainer_sprite_palettes[][4] = {
#if defined(PBL_COLOR)
    {0b11111111, 0b11111001, 0b11110000, 0b11000000},
    {0b11111111, 0b11111001, 0b11010111, 0b11000000},
    {0b11111111, 0b11111001, 0b11001000, 0b11000000},
    {0b11111111, 0b11111001, 0b11010100, 0b11000000},
    {0b11111111, 0b11111001, 0b11110101, 0b11000000},
    {0b11111111, 0b11111111, 0b11010101, 0b11000000},
#else
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 0},
#endif
};

const uint8_t pokemon_trainer_sheet_offsets[] = {
    POKEMON_SPRITE_CHRIS,
    POKEMON_SPRITE_CHRIS_BIKE,
    POKEMON_SPRITE_KRIS,
    POKEMON_SPRITE_KRIS_BIKE,
    POKEMON_SPRITE_RIVAL,
    POKEMON_SPRITE_OAK,
    POKEMON_SPRITE_RED,
    POKEMON_SPRITE_BLUE,
};