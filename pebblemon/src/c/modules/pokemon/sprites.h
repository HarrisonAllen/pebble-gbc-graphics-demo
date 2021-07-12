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

#define POKEMON_SPRITE_CUT_TREE 133
#define POKEMON_SPRITE_ITEM 132
#define POKEMON_SPRITELET_GRASS 134 // Only one tile!

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

uint8_t pokemon_trainer_palettes[][4] = {
#if defined(PBL_COLOR)
    {0b11111111, 0b11111001, 0b11110000, 0b11000000},    // Chris
    {0b11111111, 0b11111001, 0b11010111, 0b11000000},    // Kris
    {0b11111111, 0b11111001, 0b11110001, 0b11000000},    // Rival
    {0b11111111, 0b11111001, 0b11100100, 0b11000000},    // Prof. Elm
    {0b11111111, 0b11111001, 0b11100100, 0b11000000},    // Prof. Oak
    {0b11111111, 0b11111001, 0b11110111, 0b11000000},    // Nurse
    {0b11111111, 0b11111001, 0b11100100, 0b11000000},    // Team Rocket Grunt (Male)
    {0b11111111, 0b11111001, 0b11110000, 0b11000000},    // Team Rocket Grunt (Female)
    {0b11111111, 0b11111001, 0b11110000, 0b11000000},    // Mom
    {0b11111111, 0b11111001, 0b11010000, 0b11000000},    // Jasmine
    {0b11111111, 0b11111001, 0b11110100, 0b11000000},    // Misty
    {0b11111111, 0b11111001, 0b11100100, 0b11000000},    // Brock
    {0b11111111, 0b11111001, 0b11100100, 0b11000000},    // Bruno
    {0b11111111, 0b11111001, 0b11010101, 0b11000000},    // Old Man
    {0b11111111, 0b11111001, 0b11010101, 0b11000000},    // Old Woman
    {0b11111111, 0b11111001, 0b11100011, 0b11000000},    // Kimono Girl
    {0b11111111, 0b11111001, 0b11000100, 0b11000000},    // Gary (Blue)
    {0b11111111, 0b11111001, 0b11010110, 0b11000000},    // Clair
    {0b11111111, 0b11111001, 0b11100101, 0b11000000},    // Lance
    {0b11111111, 0b11111111, 0b11111011, 0b11000000},    // Clefairy
    {0b11111111, 0b11111110, 0b11110100, 0b11000000},    // Dragon
    {0b11111111, 0b11111101, 0b11110000, 0b11000000},    // Surfing Pikachu
#else
    {1, 1, 0, 0},    // Chris
    {1, 1, 0, 0},    // Kris
    {1, 1, 0, 0},    // Rival
    {1, 1, 0, 0},    // Prof. Elm
    {1, 1, 0, 0},    // Prof. Oak
    {1, 1, 0, 0},    // Nurse
    {1, 1, 0, 0},    // Team Rocket Grunt (Male)
    {1, 1, 0, 0},    // Team Rocket Grunt (Female)
    {1, 1, 0, 0},    // Mom
    {1, 1, 0, 0},    // Jasmine
    {1, 1, 0, 0},    // Misty
    {1, 1, 0, 0},    // Brock
    {1, 1, 0, 0},    // Bruno
    {1, 1, 0, 0},    // Old Man
    {1, 1, 0, 0},    // Old Woman
    {1, 1, 0, 0},    // Kimono Girl
    {1, 1, 0, 0},    // Gary (Blue)
    {1, 1, 0, 0},    // Clair
    {1, 1, 0, 0},    // Lance
    {1, 1, 0, 0},    // Clefairy
    {1, 1, 0, 0},    // Dragon
    {1, 1, 0, 0},    // Surfing Pikachu
#endif
};