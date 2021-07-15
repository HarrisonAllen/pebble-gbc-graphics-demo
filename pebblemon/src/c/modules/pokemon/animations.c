#include "animations.h"

static uint8_t s_anim_bank, s_anim_offset;
static uint8_t s_anim_frame;

uint8_t tile_water_cave_swaps[] = {
    6, 6, 7, 7, 8, 8, 9, 9, 8, 8, 7, 7, 6, 6, 7, 7, 8, 8, 9, 9, 8, 8, 7, 7, 
};
uint8_t tile_water_swaps[] = {
    2, 2, 3, 3, 4, 4, 5, 5, 2, 2, 3, 3, 4, 4, 5, 5, 2, 2, 3, 3, 4, 4, 5, 5, 
};

uint8_t tile_fountain_swaps[] = {
    11, 12, 13, 14, 15, 16, 17, 18, 11, 12, 13, 14, 15, 16, 17, 18, 11, 12, 13, 14, 15, 16, 17, 18, 
};

uint8_t tile_flower_swaps[] = {
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 
};

uint8_t anim_tile_offsets[] = {
    TILE_WATER_CAVE,
    TILE_WATER,
    TILE_FOUNTAIN,
    TILE_FLOWER
};

uint8_t *anim_tiles[] = {
    tile_water_cave_swaps,
    tile_water_swaps,
    tile_fountain_swaps,
    tile_flower_swaps
};

uint8_t cave_water_palettes[] = {
#if defined(PBL_COLOR)
    0b11011011, 0b11000111, 0b11000010, 0b11000000, 
    0b11000111, 0b11000111, 0b11000010, 0b11000000, 
    0b11000010, 0b11000111, 0b11000010, 0b11000000, 
    0b11000111, 0b11000111, 0b11000010, 0b11000000, 
#else
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
#endif
};

uint8_t day_water_palettes[] = {
#if defined(PBL_COLOR)
    0b11111111, 0b11101011, 0b11010111, 0b11000000, 
    0b11101011, 0b11101011, 0b11010111, 0b11000000, 
    0b11010111, 0b11101011, 0b11010111, 0b11000000, 
    0b11101011, 0b11101011, 0b11010111, 0b11000000, 
#else
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
#endif
};

uint8_t morning_water_palettes[] = {
#if defined(PBL_COLOR)
    0b11111110, 0b11101011, 0b11010111, 0b11000000, 
    0b11101011, 0b11101011, 0b11010111, 0b11000000, 
    0b11010111, 0b11101011, 0b11010111, 0b11000000, 
    0b11101011, 0b11101011, 0b11010111, 0b11000000, 
#else
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
#endif
};

uint8_t night_water_palettes[] = {
#if defined(PBL_COLOR)
    0b11111111, 0b11101010, 0b11010110, 0b11000000,
    0b11101010, 0b11101010, 0b11010110, 0b11000000,
    0b11010110, 0b11010111, 0b11010110, 0b11000000,
    0b11101010, 0b11101010, 0b11010110, 0b11000000,
#else
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
    0, 0, 1, 1,
#endif
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

void animate_tiles(GBC_Graphics *graphics, uint8_t tile_bank, uint8_t route) {
    for (uint8_t i = 0; i < NUM_ANIMATIONS; i++) {
        GBC_Graphics_vram_move_tiles(graphics, s_anim_bank, s_anim_offset + anim_tiles[i][s_anim_frame % NUM_ANIM_FRAMES], 
                                     tile_bank, anim_tile_offsets[i], 1, false);
    }
    if (route == 0) { // Cave
        GBC_Graphics_set_bg_palette_array(graphics, WATER_PALETTE, &cave_water_palettes[((s_anim_frame % 16) / 4) * 4]);
    } else {
        time_t unadjusted_time = time(NULL);
        struct tm *cur_time = localtime(&unadjusted_time);
        uint8_t hour = cur_time->tm_hour;
        uint8_t *palette;
        if (hour >= 4 && hour < 10) {
            palette = morning_water_palettes;
        } else if (hour >= 10 && hour < 18) {
            palette = day_water_palettes;
        } else {
            palette = night_water_palettes;
        }
        GBC_Graphics_set_bg_palette_array(graphics, WATER_PALETTE, &palette[((s_anim_frame % 8) / 2) * 4]);
    }
    s_anim_frame = (s_anim_frame + 1) % MAX_ANIM_FRAMES;
}