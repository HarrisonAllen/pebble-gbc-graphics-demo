#include "menu.h"

static GPoint s_menu_root;
static uint8_t s_cursor_pos, s_num_menu_items;
static GPoint s_dialogue_root;
static uint8_t s_dialogue_x, s_dialogue_y;
static DialogueState s_dialogue_state;
static uint8_t s_dialogue_buffer_pos;
static uint16_t s_dialogue_frame;
static uint8_t *s_dialogue_buffer;
static uint8_t s_max_lines = 2;
static uint8_t s_cur_line = 0;
static GRect s_dialogue_bounds;
static bool s_dialogue_waits;
static bool s_menu_tight;
static uint8_t s_window_frame;
static bool s_route_frame_animating;


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

void draw_uint_string_at_location(GBC_Graphics *graphics, GPoint pos, uint8_t *text, uint8_t len) {
  char to_draw[len];
  snprintf(to_draw, len, "%s", text);
  draw_text_at_location(graphics, pos, to_draw);
}

void draw_char_at_location(GBC_Graphics *graphics, GPoint pos, char char_to_write) {
  uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, pos.x, pos.y, char_to_write - MENU_ASCII_OFFSET, attrs);
}

void draw_menu(GBC_Graphics *graphics, GRect rect_bounds, char *text, bool blank, bool tight) {
  s_menu_root = rect_bounds.origin;
  s_menu_tight = tight;
  if (blank) {
    draw_blank_rectangle(graphics, rect_bounds);
  } else {
    draw_menu_rectangle(graphics, rect_bounds);
  }
  uint8_t y_offset = tight ? 1 : 2;
  draw_text_at_location(graphics, GPoint(s_menu_root.x + 2, s_menu_root.y + y_offset), text);
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
  uint8_t y_offset = s_menu_tight ? 1 : 2;
  draw_char_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + y_offset + s_cursor_pos * 2), ' ');
}

void draw_menu_cursor(GBC_Graphics *graphics) {
  uint8_t y_offset = s_menu_tight ? 1 : 2;
  draw_char_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + y_offset + s_cursor_pos * 2), '>');
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

void begin_dialogue_from_string(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, char *dialogue, bool wait) {
  if (s_route_frame_animating) {
    quit_route_frame_animation(graphics);
  }
  unload_dialogue();
  s_dialogue_buffer = (uint8_t*)malloc(strlen(dialogue)+1);
  memcpy(s_dialogue_buffer, dialogue, strlen(dialogue)+1);

  s_dialogue_bounds = dialogue_bounds;
  draw_menu_rectangle(graphics, dialogue_bounds);
  s_dialogue_root = dialogue_root;
  s_dialogue_x = dialogue_root.x;
  s_dialogue_y = dialogue_root.y;
  s_dialogue_state = D_WRITING;
  s_dialogue_buffer_pos = 0;
  s_dialogue_waits = wait;
}

void begin_dialogue(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, uint16_t dialogue_id, bool wait) {
  if (s_route_frame_animating) {
    quit_route_frame_animation(graphics);
  }
  ResHandle data_handle = resource_get_handle(RESOURCE_ID_DATA_DIALOGUE_DATA);
  uint8_t *data_buffer = (uint8_t*)malloc(4);
  resource_load_byte_range(data_handle, dialogue_id * 4, data_buffer, 4);
  uint16_t text_offset = (data_buffer[0] << 8) | data_buffer[1];
  uint16_t text_size = (data_buffer[2] << 8) | data_buffer[3];
  free(data_buffer);

  ResHandle text_handle = resource_get_handle(RESOURCE_ID_DATA_DIALOGUE_TEXT);
  unload_dialogue();
  s_dialogue_buffer = (uint8_t*)malloc(text_size);
  resource_load_byte_range(text_handle, text_offset, s_dialogue_buffer, text_size);
  s_dialogue_bounds = dialogue_bounds;
  draw_menu_rectangle(graphics, dialogue_bounds);
  s_dialogue_root = dialogue_root;
  s_dialogue_x = dialogue_root.x;
  s_dialogue_y = dialogue_root.y;
  s_dialogue_state = D_WRITING;
  s_dialogue_buffer_pos = 0;
  s_dialogue_waits = wait;
}

DialogueState get_dialogue_state() {
  return s_dialogue_state;
}

void step_dialogue(GBC_Graphics *graphics, bool select_pressed, uint8_t speed) {
  switch (s_dialogue_state) {
    case D_IDLE:
      break;
    case D_WRITING:
      if (speed == 2) {
        while (s_dialogue_state == D_WRITING) {
          step_dialogue(graphics, select_pressed, 1);
        }
      } else {
        s_dialogue_frame = (s_dialogue_frame + 1) % 2;
        if (s_dialogue_frame == 0 || select_pressed || speed == 1) {
          if (s_dialogue_buffer[s_dialogue_buffer_pos] == '\n') {
            s_dialogue_x = s_dialogue_root.x;
            s_dialogue_y += 2;
            s_cur_line++;
            if (s_cur_line >= s_max_lines) {
              s_cur_line = 0;
              s_dialogue_y = s_dialogue_root.y;
              s_dialogue_state = D_WAITING;
            }
          } else if (s_dialogue_buffer[s_dialogue_buffer_pos] == '\0'){
            s_cur_line = 0;
            if (s_dialogue_waits) {
              s_dialogue_state = D_FINAL_WAIT;
            } else {
              s_dialogue_state = D_IDLE;
            }

            free(s_dialogue_buffer);
            s_dialogue_buffer = NULL;
          } else {
            draw_char_at_location(graphics, GPoint(s_dialogue_x, s_dialogue_y), s_dialogue_buffer[s_dialogue_buffer_pos]);
            s_dialogue_x++;
          }
          s_dialogue_buffer_pos++;
        }
      }
      break;
    case D_WAITING:
      s_dialogue_frame = (s_dialogue_frame + 1) % 16;
      uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);
      if (s_dialogue_frame == 0) {
        draw_char_at_location(graphics, GPoint(s_dialogue_root.x + PBL_IF_ROUND_ELSE(13, 15), s_dialogue_root.y + 3), '<');
      } else if (s_dialogue_frame == 8) {
        GBC_Graphics_bg_set_tile_and_attrs(graphics, s_dialogue_root.x + PBL_IF_ROUND_ELSE(13, 15), s_dialogue_root.y + 3, MENU_PIPE_H, attrs);
      }
      break;
    case D_FINAL_WAIT:
      break;
  }
}

void handle_input_dialogue(GBC_Graphics *graphics) {
  switch (s_dialogue_state) {
    case D_WAITING:
      s_dialogue_state = D_WRITING;
      draw_menu_rectangle(graphics, s_dialogue_bounds);
      break;
    case D_FINAL_WAIT:
      s_dialogue_state = D_IDLE;
      break;
    default:
      break;
  }
}

void unload_dialogue() {
  if (s_dialogue_buffer != NULL) {
    free(s_dialogue_buffer);
    s_dialogue_buffer = NULL;
  }
}


void draw_enemy_battle_frame(GBC_Graphics *graphics) {
  GPoint root = GPoint(0, PBL_IF_ROUND_ELSE(1, 0));
#if defined(PBL_COLOR)
  GBC_Graphics_set_bg_palette(graphics, 3, 0b11111111, 0b11111110, 0b11000000, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, 3, 0, 0, 1, 1);
#endif
  uint8_t frame_attrs = GBC_Graphics_attr_make(3, MENU_VRAM_BANK, false, false, false);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+0, root.y+2, BATTLE_V, frame_attrs);
  GBC_Graphics_bg_set_tile_x_flip(graphics, root.x+0, root.y+2, true);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+0, root.y+3, BATTLE_SW, frame_attrs);
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+1+i, root.y+3, BATTLE_HEALTH_EMPTY, frame_attrs);
  }
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+9, root.y+3, BATTLE_TAIL, frame_attrs);
  GBC_Graphics_bg_set_tile_x_flip(graphics, root.x+9, root.y+3, true);

  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+1, root.y+2, BATTLE_H, frame_attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+2, root.y+2, BATTLE_P, frame_attrs);
  for (uint8_t i = 0; i < 6; i++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+3+i, root.y+2, BATTLE_HEALTH_EMPTY, frame_attrs);
  }
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+9, root.y+2, BATTLE_HEALTH_END, frame_attrs);
}

void draw_player_battle_frame(GBC_Graphics *graphics) {
  GPoint root = GPoint(7, 7);
#if defined(PBL_COLOR)
  GBC_Graphics_set_bg_palette(graphics, 3, 0b11111111, 0b11111110, 0b11000000, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, 3, 0, 0, 1, 1);
#endif
  uint8_t frame_attrs = GBC_Graphics_attr_make(3, MENU_VRAM_BANK, false, false, false);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+1, root.y+2, BATTLE_H, frame_attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+2, root.y+2, BATTLE_P, frame_attrs);
  for (uint8_t i = 0; i < 6; i++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+3+i, root.y+2, BATTLE_HEALTH_EMPTY, frame_attrs);
  }
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+9, root.y+2, BATTLE_NE, frame_attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+9, root.y+3, BATTLE_V, frame_attrs);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+9, root.y+4, BATTLE_SE, frame_attrs);
  for (uint8_t i = 0; i < 8; i++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+1+i, root.y+4, BATTLE_HEALTH_EMPTY, frame_attrs);
  }
  GBC_Graphics_bg_set_tile_and_attrs(graphics, root.x+0, root.y+4, BATTLE_TAIL, frame_attrs);
}

void draw_battle_frames(GBC_Graphics *graphics) {
  draw_enemy_battle_frame(graphics);
  draw_player_battle_frame(graphics);
}

static void draw_hp_bar(GBC_Graphics *graphics, uint8_t palette, GPoint bar_start, uint8_t bar_len, uint16_t max_hp, uint16_t cur_hp) {
#if defined(PBL_COLOR)
  float health_percentage = (float)cur_hp / max_hp;
  uint8_t color;
  if (health_percentage < 0.2) {
    color = HP_RED;
  } else if (health_percentage < 0.5) {
    color = HP_YELLOW;
  } else {
    color = HP_GREEN;
  }
  GBC_Graphics_set_bg_palette(graphics, palette, 0b11111111, 0b11111111, color, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, palette, 0, 0, 1, 1);
#endif
  uint8_t num_pixels = (bar_len * 8) * cur_hp / max_hp;

  uint8_t num_full_bars = num_pixels / 8;
  uint8_t last_bar_len = num_pixels % 8;
  uint8_t attrs = GBC_Graphics_attr_make(palette, MENU_VRAM_BANK, false, false, false);
  uint8_t bar_pos;
  for (bar_pos = 0; bar_pos < num_full_bars; bar_pos++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x+bar_pos, bar_start.y, BATTLE_HEALTH_FULL, attrs);
  }
  if (bar_pos < bar_len) {
    if (num_full_bars == 0 && last_bar_len == 0 && cur_hp != 0) {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x+bar_pos, bar_start.y, BATTLE_HEALTH_EMPTY+1, attrs);
    } else {
      GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x+bar_pos, bar_start.y, BATTLE_HEALTH_EMPTY+last_bar_len, attrs);
    }
  }
  for (bar_pos++; bar_pos < bar_len; bar_pos++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x+bar_pos, bar_start.y, BATTLE_HEALTH_EMPTY, attrs);
  }
}

void draw_enemy_hp_bar(GBC_Graphics *graphics, uint16_t max_health, uint16_t cur_health) {
  draw_hp_bar(graphics, 4, GPoint(3, PBL_IF_ROUND_ELSE(3, 2)), 6, max_health, cur_health);
}

void draw_player_hp_bar(GBC_Graphics *graphics, uint16_t max_health, uint16_t cur_health) {
  draw_hp_bar(graphics, 5, GPoint(10, 9), 6, max_health, cur_health);
  char player_hp[8];
  snprintf(player_hp, 8, "%3d/%3d", cur_health, max_health);
  draw_text_at_location(graphics, GPoint(9, 10), player_hp);
}

void draw_exp_bar(GBC_Graphics *graphics, int max_exp, int cur_exp) {
  uint8_t bar_len = 8;
  GPoint bar_start = GPoint(15, 11);
#if defined(PBL_COLOR)
  GBC_Graphics_set_bg_palette(graphics, 6, 0b11111111, 0b11111111, EXP_BLUE, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, 6, 0, 0, 1, 1);
#endif
  uint8_t num_pixels = (bar_len * 8) * cur_exp / max_exp;

  uint8_t num_full_bars = num_pixels / 8;
  uint8_t last_bar_len = num_pixels % 8;
  uint8_t attrs = GBC_Graphics_attr_make(6, MENU_VRAM_BANK, true, false, false);
  uint8_t bar_pos;
  for (bar_pos = 0; bar_pos < num_full_bars; bar_pos++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x-bar_pos, bar_start.y, BATTLE_HEALTH_FULL, attrs);
  }
  if (bar_pos < bar_len) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x-bar_pos, bar_start.y, BATTLE_HEALTH_EMPTY+last_bar_len, attrs);
  }
  for (bar_pos++; bar_pos < bar_len; bar_pos++) {
    GBC_Graphics_bg_set_tile_and_attrs(graphics, bar_start.x-bar_pos, bar_start.y, BATTLE_HEALTH_EMPTY, attrs);
  }
}

static void draw_route_frame(GBC_Graphics *graphics) {
  uint8_t attrs = GBC_Graphics_attr_make(6, MENU_VRAM_BANK, false, false, true);
  for (uint8_t x = 0; x < 18; x++) {
    for (uint8_t y = 0; y < 4; y++) {
      GBC_Graphics_window_set_tile_and_attrs(graphics, x, y, ROUTE_FRAME_C, attrs);
    }
  }
  for (uint8_t x = 0; x < 18; x++) {
    GBC_Graphics_window_set_tile_and_attrs(graphics, x, 0, x % 4 < 2 ? ROUTE_FRAME_N2 : ROUTE_FRAME_N1, attrs);
    GBC_Graphics_window_set_tile_and_attrs(graphics, x, 3, x % 4 < 2 ? ROUTE_FRAME_S2 : ROUTE_FRAME_S1, attrs);
  }
  GBC_Graphics_window_set_tile_and_attrs(graphics, 0, 0, ROUTE_FRAME_NW, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 0, 1, ROUTE_FRAME_W1, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 0, 2, ROUTE_FRAME_W2, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 0, 3, ROUTE_FRAME_SW, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 17, 0, ROUTE_FRAME_NE, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 17, 1, ROUTE_FRAME_E1, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 17, 2, ROUTE_FRAME_E2, attrs);
  GBC_Graphics_window_set_tile_and_attrs(graphics, 17, 3, ROUTE_FRAME_SE, attrs);
}

void begin_route_frame_animation(GBC_Graphics *graphics, char *route_name) {
#if defined(PBL_COLOR)
  GBC_Graphics_set_bg_palette(graphics, 6, 0b11111110, 0b11111110, 0b11100100, 0b11000000);
#else
  GBC_Graphics_set_bg_palette(graphics, 6, 0, 0, 1, 1);
#endif
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, true);
  GBC_Graphics_window_set_offset_y(graphics, 144);
  s_window_frame = 0;

  draw_route_frame(graphics);
  
  uint16_t num_chars = strlen(route_name);
  uint8_t x = 1 + (16 - num_chars) / 2;
  uint8_t y = 2;
  uint8_t vram_pos;
  char char_to_write;
  for (uint16_t i = 0; i < num_chars; i++) {
    char_to_write = route_name[i];
    vram_pos = char_to_write - MENU_ASCII_OFFSET;
    GBC_Graphics_window_set_tile(graphics, x, y, vram_pos);
    x++;
  }
  s_route_frame_animating = true;
}

void step_route_frame_animation(GBC_Graphics *graphics) {
  if (s_window_frame < 16) {
    GBC_Graphics_window_move(graphics, 0, -2);
  } else if (s_window_frame >= 60 && s_window_frame < 76) {
    GBC_Graphics_window_move(graphics, 0, 2);
  } else if (s_window_frame == 76) {
    quit_route_frame_animation(graphics);
  }
  s_window_frame++;
}

bool is_route_frame_animating() {
  return s_route_frame_animating;
}

void quit_route_frame_animation(GBC_Graphics *graphics) {
  GBC_Graphics_window_set_offset_y(graphics, 144);
  GBC_Graphics_lcdc_set_window_layer_enabled(graphics, false);
  s_route_frame_animating = false;
}