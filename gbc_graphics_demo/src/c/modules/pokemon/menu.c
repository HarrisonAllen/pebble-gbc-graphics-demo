#include "menu.h"

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