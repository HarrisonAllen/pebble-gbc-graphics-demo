#include "animations.h"

static uint8_t s_anim_bank, s_anim_offset;
static uint8_t s_anim_frame;

const uint8_t tile_water_cave_swaps[] = {
    6, 6, 7, 7, 8, 8, 9, 9, 8, 8, 7, 7, 6, 6, 7, 7, 8, 8, 9, 9, 8, 8, 7, 7, 
};
const uint8_t tile_water_swaps[] = {
    2, 2, 3, 3, 4, 4, 5, 5, 2, 2, 3, 3, 4, 4, 5, 5, 2, 2, 3, 3, 4, 4, 5, 5, 
};

const uint8_t tile_fountain_swaps[] = {
    11, 12, 13, 14, 15, 16, 17, 18, 11, 12, 13, 14, 15, 16, 17, 18, 11, 12, 13, 14, 15, 16, 17, 18, 
};

const uint8_t tile_flower_swaps[] = {
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 
};

const uint8_t anim_tile_offsets[] = {
    TILE_WATER_CAVE,
    TILE_WATER,
    TILE_FOUNTAIN,
    TILE_FLOWER
};

const uint8_t *anim_tiles[] = {
    tile_water_cave_swaps,
    tile_water_swaps,
    tile_fountain_swaps,
    tile_flower_swaps
};

void init_anim_tiles(GBC_Graphics *graphics, uint8_t anim_bank, uint8_t anim_offset) {
    ResHandle handle = resource_get_handle(RESOURCE_ID_DATA_ANIMATION_TILESHEET);
    size_t res_size = resource_size(handle);
    uint16_t tiles_to_load = res_size / 16;
    GBC_Graphics_load_from_tilesheet_into_vram(graphics, RESOURCE_ID_DATA_ANIMATION_TILESHEET, 0, tiles_to_load, anim_offset, anim_bank);
    s_anim_bank = anim_bank;
    s_anim_offset = anim_offset;
    s_anim_frame = 0;
}

void animate_tiles(GBC_Graphics *graphics, uint8_t tile_bank) {
    for (uint8_t i = 0; i < NUM_ANIMATIONS; i++) {
    // for (uint8_t i = 0; i < sizeof(anim_tiles) / sizeof(anim_tiles[0]); i++) {
        GBC_Graphics_vram_move_tiles(graphics, s_anim_bank, s_anim_offset + anim_tiles[i][s_anim_frame], 
                                     tile_bank, anim_tile_offsets[i], 1, false);
    }    
    // s_anim_frame = (s_anim_frame + 1) % (sizeof(anim_tiles[0]) / sizeof(anim_tiles[0][0]));
    s_anim_frame = (s_anim_frame + 1) % NUM_ANIM_FRAMES;
}