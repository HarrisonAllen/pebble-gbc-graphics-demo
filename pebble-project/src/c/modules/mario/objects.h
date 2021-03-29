#pragma once

#include <pebble.h>

#define MARIO_STAND 0
#define MARIO_STEP_0 4
#define MARIO_STEP_1 8
#define MARIO_STEP_2 12
#define MARIO_JUMP 16

const uint8_t mario_sprites[] = {
    MARIO_STAND,
    MARIO_STEP_0,
    MARIO_STEP_1,
    MARIO_STEP_2,
    MARIO_JUMP
};

typedef enum {
    SKY,
    C_B,
    BRK,
    M_B,
    PTL,
    PTR,
    PBL,
    PBR,
    CTL,
    CTM,
    CTR,
    CBL,
    CBM,
    CBR,
    B_L,
    B_M,
    B_R,
    H_T,
    H_L,
    H_R,
    H_S,
    H_B,
    S_B,
    F_T,
    FPL,
    CRS,
    CRB,
    CWL,
    CBK,
    CWR,
    CDT,
    CDB
} MarioObject;

// Basically objects are:
//    {
//      top left, bottom left,
//      top right, bottom right
//              }
// Which is the transpose of what they look like

const uint8_t sky[] = {
    57, 57,
    57, 57
};

const uint8_t cracked_block[] = {
    0, 1,
    2, 3
};

const uint8_t brick[] = {
    4, 5,
    6, 7
};

const uint8_t mystery_block[] = {
    8, 9,
    10, 11
};

const uint8_t pipe_top_left[] = {
    12, 16,
    13, 17,
};

const uint8_t pipe_top_right[] = {
    14, 18,
    15, 19
};

const uint8_t pipe_base_left[] = {
    20, 20,
    21, 21
};

const uint8_t pipe_base_right[] = {
    55, 55,
    22, 22,
};

const uint8_t cloud_top_left[] = {
    57, 57,
    57, 23, 
};

const uint8_t cloud_top_middle[] = {
    24, 56,
    25, 56
};

const uint8_t cloud_top_right[] = {
    57, 26,
    57, 57
};

const uint8_t cloud_bottom_left[] = {
    57, 57,
    27, 57
};

const uint8_t cloud_bottom_middle[] = {
    28, 57,
    29, 57
};

const uint8_t cloud_bottom_right[] = {
    30, 57,
    57, 57
};

const uint8_t bush_left[] = {
    57, 57,
    57, 23, 
};

const uint8_t bush_middle[] = {
    24, 56,
    25, 56
};

const uint8_t bush_right[] = {
    57, 26,
    57, 57
};

const uint8_t hill_top[] = {
    57, 32,
    57, 33
};

const uint8_t hill_left[] = {
    57, 31,
    31, 55
};

const uint8_t hill_right[] = {
    34, 55,
    57, 34
};

const uint8_t hill_spot[] = {
    55, 55,
    35, 55
};

const uint8_t hill_blank[] = {
    55, 55,
    55, 55
};

const uint8_t solid_block[] = {
    36, 37,
    38, 39
};

const uint8_t flagpole_top[] = {
    57, 40,
    57, 41
};

const uint8_t flagpole[] = {
    42, 42,
    43, 43
};

const uint8_t castle_ramparts_sky[] = {
    48, 5,
    49, 7
};

const uint8_t castle_ramparts_brick[] = {
    50, 5,
    51, 7
};

const uint8_t castle_window_left[] = {
    5, 5,
    54, 54
};

const uint8_t castle_brick[] = {
    5, 5,
    5, 5
};

const uint8_t castle_window_right[] = {
    54, 54,
    5, 5
};

const uint8_t castle_door_top[] = {
    52, 54,
    53, 54
};

const uint8_t castle_door_bottom[] = {
    54, 54,
    54, 54
};

const uint8_t *object_array[] = {
    sky,
    cracked_block,
    brick,
    mystery_block,
    pipe_top_left,
    pipe_top_right,
    pipe_base_left,
    pipe_base_right,
    cloud_top_left,
    cloud_top_middle,
    cloud_top_right,
    cloud_bottom_left,
    cloud_bottom_middle,
    cloud_bottom_right,
    bush_left,
    bush_middle,
    bush_right,
    hill_top,
    hill_left,
    hill_right,
    hill_spot,
    hill_blank,
    solid_block,
    flagpole_top,
    flagpole,
    castle_ramparts_sky,
    castle_ramparts_brick,
    castle_window_left,
    castle_brick,
    castle_window_right,
    castle_door_top,
    castle_door_bottom
};