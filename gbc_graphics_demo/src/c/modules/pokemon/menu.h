#pragma once

#include <pebble.h>
#include "../pebble-gbc-graphics/pebble-gbc-graphics.h"

#define MENU_VRAM_BANK 1
#define MENU_PIPE_OFFSET 98
#define MENU_PIPE_NW 98
#define MENU_PIPE_H 99
#define MENU_PIPE_NE 100
#define MENU_PIPE_V 101
#define MENU_PIPE_SW 102
#define MENU_PIPE_SE 103
#define MENU_ASCII_OFFSET ' '

// menu_bounds is in tiles
void draw_menu_rectangle(GBC_Graphics *graphics, GRect menu_bounds);
void draw_blank_rectangle(GBC_Graphics *graphics, GRect rect_bounds);
// pos is in tiles
void draw_text_at_location(GBC_Graphics *graphics, GPoint pos, char *text);
