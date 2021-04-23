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

typedef enum {
  D_IDLE,
  D_WRITING,
  D_WAITING,
  D_FINAL_WAIT
} DialogueState;

// menu_bounds is in tiles
void draw_menu_rectangle(GBC_Graphics *graphics, GRect menu_bounds);
void draw_blank_rectangle(GBC_Graphics *graphics, GRect rect_bounds);
// pos is in tiles
void draw_text_at_location(GBC_Graphics *graphics, GPoint pos, char *text);
void draw_char_at_location(GBC_Graphics *graphics, GPoint pos, char char_to_write);
void draw_menu(GBC_Graphics *graphics, GRect rect_bounds, char *text, bool blank);
void draw_textbox(GBC_Graphics *graphics, GRect rect_bounds, GPoint offset, char *text, bool blank);

void erase_menu_cursor(GBC_Graphics *graphics);
void draw_menu_cursor(GBC_Graphics *graphics);
void set_menu_root(GPoint menu_root);
void set_cursor_pos(uint8_t cursor_pos);
void set_num_menu_items(uint8_t num_menu_items);
uint8_t get_cursor_pos();
void move_cursor_down(GBC_Graphics *graphics);
void move_cursor_up(GBC_Graphics *graphics);

void begin_dialogue_from_string(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, char *dialogue);
void begin_dialogue(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, uint16_t dialogue_id);
DialogueState get_dialogue_state();
void step_dialogue(GBC_Graphics *graphics, bool select_pressed, uint8_t frame_delay);
void handle_input_dialogue(GBC_Graphics *graphics);
void unload_dialogue();