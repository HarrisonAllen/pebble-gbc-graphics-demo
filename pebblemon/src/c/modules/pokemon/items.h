#pragma once

#include <pebble.h>

#define ITEM_ID_RUNNING_SHOES 0 // Running Shoes (double move speed) - Forest, top half
#define ITEM_ID_CUT 1           // Cut (enables cutting of trees) - Forest, bottom half
#define ITEM_ID_LUCKY_EGG 2     // Lucky Egg (double exp) - National Park, top right
#define ITEM_ID_BERRY 3         // Berry (heals 10HP when below half, one time use) - Berry tree route 1, can come back for more
#define ITEM_ID_LEFTOVERS 4     // Leftovers (restore a 1/16 HP each turn) - Far right trash can National Park
#define ITEM_ID_FOCUS_BAND 5    // Focus Band (12% chance to prevent fainting) - Cave, in path 
#define ITEM_ID_PROTEIN 6       // Protein (Raises attack stat by 25% each battle) - Route 1, top right
#define ITEM_ID_IRON 7          // Iron (Raises defense stat by 25% each battle) - Cave, on stairs

#define GET_ITEM_FLAG(id) 1 << (id)
#define HAS_ITEM(data, id) (data) & (1 << (id)) // Use to check if an item is obtained
#define SET_ITEM(data, id) (data) | (1 << (id)) // Set data equal to this to set an item as obtained

const int16_t items[][6] = {
    // route, block_x, block_y, item_id, sprite, hide
    {1, 19, 18, 0, 0, false}, // Running Shoes (double move speed) - Forest, top half
    {1, 29, 26, 0, 0, false}, // Cut (enables cutting of trees) - Forest, bottom half
    {2, 37, 15, 0, 0, false}, // Lucky Egg (double exp) - National Park, top right
    {0, 12, 16, 3, 0, false}, // Berry (heals 10HP when below half, one time use(?)) - Berry tree route 1, can come back for more(?)
    {2, 32, 42, 0, 0, false}, // Leftovers (restore a 1/16 HP each turn) - Far right trash can National Park
    {0, 19, 23, 0, 0, false}, // Focus Band (12% chance to prevent fainting) - Cave, in path 
    {0, 48, 16, 6, 0, false}, // Protein (Raises attack stat by 25% each battle) - Route 1, top right
    {0, 7, 21, 0, 0, false}, // Iron (Raises defense stat by 25% each battle) - Cave, on stairs
};