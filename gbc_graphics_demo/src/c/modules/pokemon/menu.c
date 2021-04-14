#include "menu.h"

GPoint s_menu_root;
uint8_t s_cursor_pos, s_num_menu_items;


void draw_menu_rectangle(GBC_Graphics *graphics, GRect menu_bounds) {
  uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);
  uint8_t root_x = menu_bounds.origin.x;
  uint8_t root_y = menu_bounds.origin.y;

  GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x, root_y, MENU_PIPE_NW, attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + menu_bounds.size.w - 1, root_y, MENU_PIPE_NE, attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x, root_y + menu_bounds.size.h - 1, MENU_PIPE_SW, attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + menu_bounds.size.w - 1, root_y + menu_bounds.size.h - 1, MENU_PIPE_SE, attrs);
  
  for (uint8_t x = 1; x < menu_bounds.size.w - 1; x++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + x, root_y, MENU_PIPE_H, attrs);
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + x, root_y + menu_bounds.size.h - 1, MENU_PIPE_H, attrs);
  }

  for (uint8_t y = 1; y < menu_bounds.size.h - 1; y++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x, root_y + y, MENU_PIPE_V, attrs);
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + menu_bounds.size.w - 1, root_y + y, MENU_PIPE_V, attrs);
  }

  for (uint8_t y = 1; y < menu_bounds.size.h - 1; y++) {
    for (uint8_t x = 1; x < menu_bounds.size.w - 1; x++) {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + x, root_y + y, 0, attrs);
    }
  }
}

void draw_blank_rectangle(GBC_Graphics *graphics, GRect rect_bounds) {
  uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);
  uint8_t root_x = rect_bounds.origin.x;
  uint8_t root_y = rect_bounds.origin.y;

  for (uint8_t y = 0; y < rect_bounds.size.h; y++) {
    for (uint8_t x = 0; x < rect_bounds.size.w; x++) {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, root_x + x, root_y + y, 0, attrs);
    }
  }
}

void draw_text_at_location(GBC_Graphics *graphics, GPoint pos, char *text) {
  uint8_t x = pos.x;
  uint8_t y = pos.y;
  uint16_t num_chars = strlen(text);
  uint8_t vram_pos;
  char char_to_write;
  uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);

  for (uint16_t i = 0; i < num_chars; i++) {
    char_to_write = text[i];
    if (char_to_write == '\n') {
      x = pos.x;
      y++;
    } else {
      vram_pos = char_to_write - MENU_ASCII_OFFSET;
      GBC_Graphics_bg_set_tile_and_attrs(graphics, x, y, vram_pos, attrs);
      x++;
    }
  }
}

void draw_menu(GBC_Graphics *graphics, GRect rect_bounds, char *text, bool blank) {
  s_menu_root = rect_bounds.origin;
  if (blank) {
    draw_blank_rectangle(graphics, rect_bounds);
  } else {
    draw_menu_rectangle(graphics, rect_bounds);
  }
  draw_text_at_location(graphics, GPoint(s_menu_root.x + 2, s_menu_root.y + 2), text);
  draw_menu_cursor(graphics);
}

void draw_textbox(GBC_Graphics *graphics, GRect rect_bounds, GPoint offset, char *text, bool blank) {
  if (blank) {
    draw_blank_rectangle(graphics, rect_bounds);
  } else {
    draw_menu_rectangle(graphics, rect_bounds);
  }
  draw_text_at_location(graphics, GPoint(rect_bounds.origin.x + offset.x, rect_bounds.origin.y + offset.y), text);
}

void erase_menu_cursor(GBC_Graphics *graphics) {
  draw_text_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + 2 + s_cursor_pos * 2), " ");
}

void draw_menu_cursor(GBC_Graphics *graphics) {
  draw_text_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + 2 + s_cursor_pos * 2), ">");
}

void set_menu_root(GPoint menu_root) {
  s_menu_root = menu_root;
}

void set_cursor_pos(uint8_t cursor_pos) {
  s_cursor_pos = cursor_pos;
}

uint8_t get_cursor_pos() {
  return s_cursor_pos;
}

void set_num_menu_items(uint8_t num_menu_items) {
  s_num_menu_items = num_menu_items;
}

void move_cursor_down(GBC_Graphics *graphics) {
  erase_menu_cursor(graphics);
  s_cursor_pos = (s_cursor_pos + 1) % s_num_menu_items;
  draw_menu_cursor(graphics);
}

void move_cursor_up(GBC_Graphics *graphics) {
  erase_menu_cursor(graphics);
  s_cursor_pos = s_cursor_pos == 0 ? s_num_menu_items - 1 : s_cursor_pos - 1;
  draw_menu_cursor(graphics);
}