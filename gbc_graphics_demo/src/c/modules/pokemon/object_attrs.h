#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

const uint8_t attr_dirt[] = {
    0 | ATTR_VRAM_BANK_01_FLAG, 0 | ATTR_VRAM_BANK_01_FLAG,
    0 | ATTR_VRAM_BANK_01_FLAG, 0 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_grass[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_tall_grass[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_cliff_W[] = {
    5 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    5 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_cliff_SW[] = {
    5 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    5 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_cliff_S[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    5 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_cliff_SE[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG,
    5 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_cliff_E[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 5 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_shrub[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_tree_top[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t attr_tree_bottom[] = {
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG,
    2 | ATTR_VRAM_BANK_01_FLAG, 2 | ATTR_VRAM_BANK_01_FLAG
};

const uint8_t *Pokemon_attr_object_array[] = {
    attr_dirt,
    attr_grass,
    attr_tall_grass,
    attr_cliff_W,
    attr_cliff_SW,
    attr_cliff_S,
    attr_cliff_SE,
    attr_cliff_E,
    attr_shrub,
    attr_tree_top,
    attr_tree_bottom
};