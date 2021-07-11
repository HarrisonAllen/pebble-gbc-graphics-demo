#pragma once

#include <pebble.h>

#define NUM_ITEMS 8
#define ITEM_ID_RUNNING_SHOES 0 // Running Shoes (double move speed) - Forest, top half
#define ITEM_ID_CUT 1           // Cut (enables cutting of trees) - Forest, bottom half
#define ITEM_ID_LUCKY_EGG 2     // Lucky Egg (double exp) - National Park, top right
#define ITEM_ID_BERRY 3         // Berry (heals 10HP when below half, once per battle) - Berry tree route 1
#define ITEM_ID_LEFTOVERS 4     // Leftovers (restore 1/16 HP each turn) - Far right trash can National Park
#define ITEM_ID_FOCUS_BAND 5    // Focus Band (12% chance to prevent fainting) - Cave, in path 
#define ITEM_ID_PROTEIN 6       // Protein (Raises attack stat by 25% each battle) - Route 1, top right
#define ITEM_ID_IRON 7          // Iron (Raises defense stat by 25% each battle) - Cave, on stairs

#define ITEM_SPRITE_POKEBALL 566

#define HAS_ITEM(data, id) ((data) & (1 << (id))) // Use to check if an item is obtained
#define SET_ITEM(data, id) ((data) | (1 << (id))) // Set data equal to this to set an item as obtained

// route, block_x, block_y, item_id, hidden
const int16_t items[][6] = {
    {1, 19, 18, ITEM_ID_RUNNING_SHOES, false},
    {1, 29, 26, ITEM_ID_CUT, false},
    {2, 37, 15, ITEM_ID_LUCKY_EGG, false},
    {3, 12, 6, ITEM_ID_BERRY, false},
    {2, 32, 42, ITEM_ID_LEFTOVERS, true},
    {0, 19, 23, ITEM_ID_FOCUS_BAND, false},
    {3, 48, 6, ITEM_ID_PROTEIN, false},
    {0, 7, 21, ITEM_ID_IRON, false},
};

char *item_names[] = {
    "RUNNING SHOES ",
    "HM01 CUT      ",
    "LUCKY EGG     ",
    "BERRY         ",
    "LEFTOVERS     ",
    "FOCUS BAND    ",
    "PROTEIN       ",
    "IRON          "
};

char *item_infos[] = {
    "      Speed up",
    "     Cut trees",
    "        EXP up",
    "   Low HP heal",
    "     Turn heal",
    "    Faint less",
    "     Attack up",
    "    Defense up"
};