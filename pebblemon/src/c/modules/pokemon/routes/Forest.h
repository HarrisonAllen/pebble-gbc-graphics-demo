#pragma once

#include <pebble.h>

uint8_t forest_chunks[] = {
    0,2,1,3,
    4,6,5,7,
    8,10,9,11,
    12,14,13,15,
    6,6,7,7,
    10,10,16,11,
    14,14,15,15,
    17,18,17,17,
    17,17,18,18,
    19,19,20,20,
    18,18,17,17,
    17,17,18,17,
    17,17,17,18,
    17,18,18,18,
    18,18,18,18,
    18,17,17,17,
    21,18,22,23,
    18,17,18,17,
    18,18,23,18,
    18,18,23,23,
    18,24,23,25,
    18,18,17,18,
    17,18,17,18,
    18,17,18,18,
    6,26,7,27,
    10,28,16,29,
    14,30,15,31,
    18,18,18,32,
    18,18,18,17,
    33,18,33,18,
    34,36,35,37,
    35,37,35,37,
    36,36,37,37,
    37,37,37,37,
    37,38,37,38,
    39,18,40,18,
    18,18,41,17,
    36,42,37,38,
    21,18,21,18,
};

uint8_t forest_blocks[] = {
    65,67,152,153,
    66,68,70,72,
    73,75,154,155,
    74,76,78,80,
    34,36,35,37,
    35,37,35,37,
    36,36,37,37,
    37,37,37,37,
    35,37,38,39,
    40,41,40,42,
    37,37,39,39,
    41,41,42,41,
    40,41,40,41,
    40,41,43,44,
    41,41,41,41,
    41,41,44,44,
    41,41,41,42,
    11,13,15,16,
    32,32,32,32,
    32,32,32,146,
    146,32,32,32,
    18,32,18,32,
    18,32,20,8,
    32,32,8,8,
    32,9,32,9,
    32,9,8,10,
    36,45,37,46,
    37,46,37,46,
    37,46,39,47,
    41,48,42,48,
    41,48,41,48,
    41,48,44,49,
    156,158,157,159,
    33,33,33,33,
    17,8,9,130,
    9,130,9,130,
    8,8,130,130,
    130,130,130,130,
    130,18,130,18,
    34,45,38,47,
    157,159,32,32,
    92,94,93,95,
    8,21,130,18,
};

uint8_t forest_block_types[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	3,
	0,
	0,
	3,
	0,
	0,
	3,
	3,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	5,
	0,
	4,
	6,
	0,
	0,
	0,
	0,
	3,
	3,
	3,
	3,
	1,
	0,
	7,
	7,
	7,
	7,
	3,
	3,
	3,
	0,
};

uint8_t forest_tile_palettes[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	5,
	5,
	5,
	5,
	2,
	2,
	2,
	2,
	2,
	2,
	5,
	5,
	5,
	5,
	5,
	2,
	0,
	5,
	5,
	5,
	5,
	0,
	0,
	0,
	0,
	2,
	1,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	1,
	4,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	2,
	2,
	2,
	2,
	2,
	2,
	5,
	5,
	2,
	2,
	2,
	2,
	5,
	5,
	2,
	2,
	5,
	5,
	5,
	5,
	5,
	0,
	0,
	5,
	5,
	5,
	5,
	2,
	2,
	2,
	2,
	0,
	0,
	0,
	0,
	0,
	0,
	2,
	2,
	2,
	2,
	5,
	5,
	5,
	5,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	4,
	5,
	4,
	5,
	0,
	3,
	0,
	3,
	3,
	3,
	3,
	3,
	0,
	0,
	0,
	0,
	0,
	0,
	5,
	5,
	5,
	1,
	5,
	5,
	5,
	5,
	5,
	2,
	2,
	2,
	2,
	5,
	5,
	5,
	5,
	0,
	0,
};