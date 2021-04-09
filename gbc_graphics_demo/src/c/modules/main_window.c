#include "main_window.h"
#include "pebble-gbc-graphics/pebble-gbc-graphics.h"
#include "mario/mario.h"
#include "pokemon/pokemon.h"

static Window *s_window;
static GBC_Graphics *s_graphics;
static AppTimer *s_frame_timer;
static uint16_t s_frame_counter;
char s_frame_buffer[5] = {0};
static Layer *s_background_layer;

typedef enum {
  DM_MARIO,
  DM_POKEMON,
  DM_END
} DemoMode;
static DemoMode s_demo_mode = DM_POKEMON;

void print_array(uint8_t* x, uint16_t len, uint16_t breakpoint);

static void next_demo();

static void load_demo(DemoMode demo) {
  switch(demo) {
    case DM_MARIO:
      Mario_initialize(s_graphics, next_demo);
      break;
    case DM_POKEMON:
      Pokemon_initialize(s_graphics, s_background_layer, next_demo);
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

static void next_demo() {
  unload_demo(s_demo_mode);
  s_demo_mode = (s_demo_mode + 1) % DM_END;
  load_demo(s_demo_mode);
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
      Pokemon_handle_select_click(s_graphics);
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
      Pokemon_handle_up(s_graphics);
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
      Pokemon_handle_back(s_graphics);
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

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);
  
  s_background_layer = layer_create(bounds);
  // layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_get_root_layer(window), s_background_layer);
  layer_mark_dirty(s_background_layer);

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
  // s_fps_timer = app_timer_register(FPS_TIMER_DELAY, fps_timer_handle, NULL);

  // layer_mark_dirty(s_foreground_layer);
  // layer_mark_dirty(s_player_ui_layer);
}

static void window_unload(Window *window) {
  GBC_Graphics_destroy(s_graphics);

  unload_demo(s_demo_mode);

  layer_destroy(s_background_layer);

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