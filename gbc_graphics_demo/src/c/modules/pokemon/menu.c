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

void draw_char_at_location(GBC_Graphics *graphics, GPoint pos, char char_to_write) {
  uint8_t attrs = GBC_Graphics_attr_make(0, MENU_VRAM_BANK, false, false, true);
  GBC_Graphics_bg_set_tile_and_attrs(graphics, pos.x, pos.y, char_to_write - MENU_ASCII_OFFSET, attrs);
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
  draw_char_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + 2 + s_cursor_pos * 2), ' ');
}

void draw_menu_cursor(GBC_Graphics *graphics) {
  draw_char_at_location(graphics, GPoint(s_menu_root.x + 1, s_menu_root.y + 2 + s_cursor_pos * 2), '>');
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

/*
  TODO:
    -Implement dialogue
      -Write characters one at a time
      -After two newlines, prompt for user input
      -Repeat until null character is reached
    -Implement dialogue storage
      -In memory, have a mapping of object to dialogue id
      -In first file, go to dialogue id offset * 4 bytes, load 4 bytes
        -Bytes 0, 1 correspond to dialogue sheet offset (uint16_t)
        -Bytes 2, 3 correspond to dialogue length (uint16_t)
      -In second file, read out 2,3 bytes from 0,1 
      -Load dialogue into s_dialogue_buffer (malloc)
      -After text is complete, free that sucker
    -Implement unload of dialogue
      -free if not null, need to call when pokemon is cleared
    -Convert a text file into a binary file
      -Need this cuz writing '\n' in a text file will convert to '\\n' when reading it
      -So go through in python, convert newline to '\0' and '\\n' to '\n'
        -Maybe do this by readline, string.replace('\\n', '\n'), 
        
static GPoint s_dialogue_root;
typedef enum {
  D_IDLE,
  D_WRITING,
  D_WAITING
} DialogueState;
static s_dialogue_state;
static uint8_t s_dialogue_buffer_pos;
static char *s_dialogue_buffer;
*/

void begin_dialogue_from_string(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, char *dialogue) {
  s_dialogue_buffer = (uint8_t*)malloc(strlen(dialogue)+1);
  memcpy(s_dialogue_buffer, dialogue, strlen(dialogue)+1);

  s_dialogue_bounds = dialogue_bounds;
  draw_menu_rectangle(graphics, dialogue_bounds);
  s_dialogue_root = dialogue_root;
  s_dialogue_x = dialogue_root.x;
  s_dialogue_y = dialogue_root.y;
  s_dialogue_state = D_WRITING;
  s_dialogue_buffer_pos = 0;
}

void begin_dialogue(GBC_Graphics *graphics, GRect dialogue_bounds, GPoint dialogue_root, uint16_t dialogue_id) {
  ResHandle data_handle = resource_get_handle(RESOURCE_ID_DATA_POKEMON_DIALOGUE_DATA);
  uint8_t *data_buffer = (uint8_t*)malloc(4);
  resource_load_byte_range(data_handle, dialogue_id * 4, data_buffer, 4);
  uint16_t text_offset = (data_buffer[0] << 8) | data_buffer[1];
  uint16_t text_size = (data_buffer[2] << 8) | data_buffer[3];
  free(data_buffer);

  ResHandle text_handle = resource_get_handle(RESOURCE_ID_DATA_POKEMON_DIALOGUE_TEXT);
  s_dialogue_buffer = (uint8_t*)malloc(text_size);
  resource_load_byte_range(text_handle, text_offset, s_dialogue_buffer, text_size);
  s_dialogue_bounds = dialogue_bounds;
  draw_menu_rectangle(graphics, dialogue_bounds);
  s_dialogue_root = dialogue_root;
  s_dialogue_x = dialogue_root.x;
  s_dialogue_y = dialogue_root.y;
  s_dialogue_state = D_WRITING;
  s_dialogue_buffer_pos = 0;
}

DialogueState get_dialogue_state() {
  return s_dialogue_state;
}

void step_dialogue(GBC_Graphics *graphics, bool select_pressed, uint8_t frame_delay) {
  switch (s_dialogue_state) {
    case D_IDLE:
      break;
    case D_WRITING:
      s_dialogue_frame = (s_dialogue_frame + 1) % frame_delay;
      if (s_dialogue_frame == 0 || select_pressed) {
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
          s_dialogue_state = D_FINAL_WAIT;
          free(s_dialogue_buffer);
          s_dialogue_buffer = NULL;
        } else {
          draw_char_at_location(graphics, GPoint(s_dialogue_x, s_dialogue_y), s_dialogue_buffer[s_dialogue_buffer_pos]);
          s_dialogue_x++;
        }
        s_dialogue_buffer_pos++;
      }
      GBC_Graphics_render(graphics);
      break;
    case D_FINAL_WAIT:
    case D_WAITING:
      // TODO: Add blinky thing at bottom
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