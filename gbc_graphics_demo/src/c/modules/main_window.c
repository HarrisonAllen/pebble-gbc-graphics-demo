#include "main_window.h"
#include "pebble-gbc-graphics/pebble-gbc-graphics.h"
#include "mario/mario.h"
#include "pokemon/pokemon.h"

static Window *s_window;
static GBC_Graphics *s_graphics;
static bool s_scroll, s_show_window=true;
static int s_state;
static uint8_t s_scroll_speed = 0;
static Layer *s_fps_counter_layer;
static uint16_t s_last_frame_time;
static uint16_t s_frame_times[50];
static uint8_t s_frame_index;
static bool s_frame_counter_enabled = true;
static AppTimer *s_frame_timer, *s_fps_timer;
static uint16_t s_frame_counter;
char s_frame_buffer[5] = {0};
static uint8_t s_counter;

static uint8_t screen_bounds[][2] = {
  {0, 168},
  {4, 160},
  {12, 144},
  {20, 128},
  {28, 112},
  {36, 96}
};

typedef enum {
  DM_START,
  DM_MARIO,
  DM_POKEMON,
  DM_END
} DemoMode;
static DemoMode s_demo_mode = DM_MARIO;

void print_array(uint8_t* x, uint16_t len, uint16_t breakpoint);

static void load_demo(DemoMode demo) {
  switch(demo) {
    case DM_MARIO:
      Mario_initialize(s_graphics);
      break;
    case DM_POKEMON:
      Pokemon_initialize(s_graphics);
    default:
      break;
  }
}

static void unload_demo(DemoMode demo) {
  switch(demo) {
    case DM_MARIO:
      Mario_deinitialize(s_graphics);
      break;
    case DM_POKEMON:
      Pokemon_deinitialize(s_graphics);
    default:
      break;
  }
}

uint16_t averaging_filter(uint16_t input, uint16_t* stored_values, uint8_t order, uint8_t* index){
    stored_values[*index] = input;
    uint16_t output = 0;
    float multiplier = 1.0/(order+1);
    for (int i=0; i<=order; i++) {
          output += stored_values[i];
    } 
    (*index)+=1;
    (*index)%=(order+1);
    return (uint16_t)(multiplier*output);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_select(s_graphics);
      break;
    case DM_POKEMON:
      break;
    default:
      break;
  }
}

static void select_press_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_POKEMON:
      Pokemon_handle_select(s_graphics, true);
      break;
    default:
      break;
  }
}

static void select_release_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      break;
    case DM_POKEMON:
      Pokemon_handle_select(s_graphics, false);
      break;
    default:
      break;
  }
}

static void up_press_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_up(s_graphics, true);
      break;
    default:
      break;
  }
}

static void up_release_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_up(s_graphics, false);
      break;
    default:
      break;
  }
}

static void down_press_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_down(s_graphics, true);
      break;
    default:
      break;
  }
}

static void down_release_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_down(s_graphics, false);
      break;
    default:
      break;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_up_click(s_graphics);
      break;
    case DM_POKEMON:
      break;
    default:
      break;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_down_click(s_graphics);
      break;
    case DM_POKEMON:
      Pokemon_handle_down(s_graphics);
      break;
    default:
      break;
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_back(s_graphics);
      break;
    case DM_POKEMON:
      break;
    default:
      break;
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_handle_tap(s_graphics);
      break;
    case DM_POKEMON:
      Pokemon_handle_tap(s_graphics);
      break;
    default:
      break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);

  // window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_press_handler, select_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_UP, up_press_handler, up_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, down_press_handler, down_release_handler, NULL);

  // window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 30, select_click_handler);
  // window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 30, down_click_handler);
  // window_single_repeating_click_subscribe(BUTTON_ID_UP, 30, up_click_handler);
}

static void nudge_bg() {
  GBC_Graphics_bg_set_scroll_x(s_graphics, GBC_Graphics_stat_get_current_line(s_graphics) % 8);
}

static void palette_swap() {
  switch((GBC_Graphics_stat_get_current_line(s_graphics) / 8) % 4) {
    case 0:
      GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11000000, 0b11010101, 0b11101010, 0b11111111);
      break;
    case 1:
      GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
      break;
    case 2:
      GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11111111, 0b11110000, 0b11001100, 0b11000011);
      break;
    case 3:
      GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11000011, 0b11001100, 0b11110000, 0b11111111);
      break;
  }
}

static void change_palette_midframe() {
  GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11111111, 0b11110000, 0b11001100, 0b11000011);  
}

static void reset_palette() {
  GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11000000, 0b11010101, 0b11101010, 0b11111111);
}

static void handle_frame() {

}

static void frame_timer_handle(void* context) {
  switch(s_demo_mode) {
    case DM_MARIO:
      Mario_step(s_graphics);
      break;
    case DM_POKEMON:
      Pokemon_step(s_graphics);
      break;
    default:
      break;
  }

  s_frame_counter++;

  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
}

static void fps_timer_handle(void *context) {
  snprintf(s_frame_buffer, 4, "%3d", s_frame_counter);
  Mario_write_string_to_background(s_graphics, 15, 0, 6, s_frame_buffer, 0);
  s_frame_counter = 0;
  GBC_Graphics_render(s_graphics);

  s_fps_timer = app_timer_register(FPS_TIMER_DELAY, fps_timer_handle, NULL);
}

static void will_focus_handler(bool in_focus) {
  if (!in_focus) {
    app_timer_cancel(s_frame_timer);
    switch(s_demo_mode) {
      case DM_MARIO:
        Mario_handle_focus_lost(s_graphics);
        break;
      case DM_POKEMON:
        Pokemon_handle_focus_lost(s_graphics);
        break;
      default:
        break;
    }
  } else {
    s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
  }
}

// void initialize_test_pattern() {
//   GBC_Graphics_load_from_tilesheet_into_vram(s_graphics, RESOURCE_ID_DATA_BASIC_COLORS_TILESHEET, 0, 4, 0, 0);
//   for (uint16_t i = 0; i < TILEMAP_SIZE; i++) {
//     s_graphics->bg_tilemap[i] = i % 3;
//     s_graphics->bg_attrmap[i] = 0;
//     s_graphics->window_tilemap[i] = i%4;
//     s_graphics->window_attrmap[i] = 2;
//   }
//   GBC_Graphics_set_bg_palette(s_graphics, 0, 0b11000000, 0b11010101, 0b11101010, 0b11111111);
//   GBC_Graphics_set_bg_palette(s_graphics, 1, 0b11111111, 0b11101010, 0b11010101, 0b11000000);
//   GBC_Graphics_set_bg_palette(s_graphics, 2, 0b11111111, 0b11110000, 0b11001100, 0b11000011);
//   GBC_Graphics_set_bg_palette(s_graphics, 3, 0b11000011, 0b11001100, 0b11110000, 0b11111111);
//   s_graphics->window_offset_y = 144;

//   GBC_Graphics_load_from_tilesheet_into_vram(s_graphics, RESOURCE_ID_DATA_BASIC_SPRITESHEET, 0, 8, 0, 1);
//   GBC_Graphics_set_sprite_palette(s_graphics, 0, 0b00000000, 0b11110000, 0b11001100, 0b11000011);
//   GBC_Graphics_set_sprite_palette(s_graphics, 1, 0b00000000, 0b11001100, 0b11111111, 0b11000000);
//   GBC_Graphics_set_sprite_palette(s_graphics, 2, 0b00000000, 0b11001100, 0b11001000, 0b11000000);
//   GBC_Graphics_set_sprite_palette(s_graphics, 3, 0b00000000, 0b11111000, 0b11110000, 0b11000000);
//   GBC_Graphics_oam_set_sprite(s_graphics, 0, 8, 16, 2, 0 | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 1, 16, 16, 2, 0 | ATTR_FLIP_FLAG_X | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 2, 15, 21, 0, 1 | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 3, 23, 21, 0, 1 | ATTR_FLIP_FLAG_X | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 20, 52, 73, 4, 2 | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 21, 60, 73, 5, 2 | ATTR_VRAM_BANK_01_FLAG | ATTR_FLIP_FLAG_Y);
//   GBC_Graphics_oam_set_sprite(s_graphics, 39, 99, 101, 6, 3 | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 5, 0, 0, 0, 1 | ATTR_VRAM_BANK_01_FLAG);
//   GBC_Graphics_oam_set_sprite(s_graphics, 5, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0 | ATTR_VRAM_BANK_01_FLAG);
//   s_graphics->lcdc &= ~LCDC_SPRITE_SIZE_FLAG;
//   GBC_Graphics_set_hblank_interrupt_callback(s_graphics, nudge_bg);
//   GBC_Graphics_stat_set_hblank_interrupt_enabled(s_graphics, true);
//   GBC_Graphics_stat_set_line_y_compare(s_graphics, 70);
//   GBC_Graphics_set_line_compare_interrupt_callback(s_graphics, change_palette_midframe);
//   GBC_Graphics_set_vblank_interrupt_callback(s_graphics, reset_palette);
//   GBC_Graphics_stat_set_line_compare_interrupt_enabled(s_graphics, true);
//   GBC_Graphics_stat_set_vblank_interrupt_enabled(s_graphics, true);
// }

static void fps_counter_update_proc(Layer *layer, GContext *ctx) {
  if (!s_frame_counter_enabled) return;
  uint16_t cur_time = time_ms(NULL, NULL);
  uint16_t time_dif = cur_time - s_last_frame_time;
  uint16_t fps = 1000 / time_dif;
  static char s_buffer[5];
  s_last_frame_time = cur_time;
  snprintf(s_buffer, 5, "%d", averaging_filter(fps, s_frame_times, 10, &s_frame_index));
  // graphics_context
  GRect rect_bounds = GRect(0, 152, 16, 16);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  // Set the color
  graphics_context_set_text_color(ctx, GColorBlack);

  // Determine a reduced bounding box
  GRect bounds = GRect(0, 152, 16, 16);

  // Calculate the size of the text to be drawn, with restricted space
  GSize text_size = graphics_text_layout_get_content_size(s_buffer, font, bounds,
                                GTextOverflowModeWordWrap, GTextAlignmentLeft);

  graphics_draw_text(ctx, s_buffer, font, bounds, GTextOverflowModeWordWrap, 
                                            GTextAlignmentLeft, NULL);

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_graphics = GBC_Graphics_ctor(window);
  load_demo(s_demo_mode);
  // print_array(s_graphics->bg_tilemap, TILEMAP_SIZE, 32);
  // print_array(s_graphics->bg_attrmap, ATTRMAP_SIZE, 32);
  // print_array(s_graphics->vram, TILE_SIZE * 64, TILE_SIZE);
  // initialize_test_pattern();
  // s_fps_counter_layer = layer_create(bounds);
  // layer_set_update_proc(s_fps_counter_layer, fps_counter_update_proc);

  // Add to Window
  // layer_add_child(window_get_root_layer(window), s_fps_counter_layer);
  
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
  s_fps_timer = app_timer_register(FPS_TIMER_DELAY, fps_timer_handle, NULL);

  // layer_mark_dirty(s_foreground_layer);
  // layer_mark_dirty(s_player_ui_layer);
}

static void window_unload(Window *window) {
  GBC_Graphics_destroy(s_graphics);

  unload_demo(s_demo_mode);

  // layer_destroy(s_fps_counter_layer);

  window_destroy(s_window);
}

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    accel_tap_service_subscribe(accel_tap_handler);
    app_focus_service_subscribe(will_focus_handler);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}

void print_array(uint8_t* x, uint16_t len, uint16_t breakpoint) {
    char* print_holder = (char*)malloc(breakpoint*3);
    memset(print_holder, 0, sizeof(print_holder));
    char* hex_array = (char*)malloc(4);
    memset(hex_array, 0, sizeof(hex_array));
    for (uint16_t i = 0; i < len; i++) {
        if (i % breakpoint == 0) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, print_holder);
          memset(print_holder, 0, sizeof(print_holder));
        }
        snprintf(hex_array, 3, "%02x ", x[i]);
        strcat(print_holder, hex_array);
    }
    if (strlen(print_holder) != 0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, print_holder);
    }
    free(print_holder);
    free(hex_array);
}