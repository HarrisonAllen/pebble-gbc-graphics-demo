#include "main_window.h"
#include "pebble-gbc-graphics/pebble-gbc-graphics.h"
#include "pokemon/pokemon.h"

static Window *s_window;
static GBC_Graphics *s_graphics;
static AppTimer *s_frame_timer;
static Layer *s_background_layer;

/* Game loading handlers */
static void load_game() {
  Pokemon_initialize(s_graphics, s_background_layer);
}

static void unload_game() {
  Pokemon_deinitialize(s_graphics);
}

/* Input handlers*/
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_select_click(s_graphics);
}

static void select_press_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_select(s_graphics, true);
}

static void select_release_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_select(s_graphics, false);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_up(s_graphics);
}

static void up_press_handler(ClickRecognizerRef recognizer, void *context) {
  
}

static void up_release_handler(ClickRecognizerRef recognizer, void *context) {
  
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_down(s_graphics);
}

static void down_press_handler(ClickRecognizerRef recognizer, void *context) {
  
}

static void down_release_handler(ClickRecognizerRef recognizer, void *context) {
  
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_back(s_graphics);
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {

}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);

  window_raw_click_subscribe(BUTTON_ID_SELECT, select_press_handler, select_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_UP, up_press_handler, up_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, down_press_handler, down_release_handler, NULL);
}

static void frame_timer_handle(void* context) {
  Pokemon_step(s_graphics);

  // Here, I draw every frame to achieve a consistent frame rate
  // However, it's possible to just draw when necessary for
  // better battery life and faster frame rates
  GBC_Graphics_render(s_graphics);

  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
}

static void will_focus_handler(bool in_focus) {
  if (!in_focus) {
    // If a notification pops up while the timer is firing
    // very rapidly, it will crash the entire watch :)
    // Stopping the timer when a notification appears will
    // prevent this while also pausing the gameplay
    app_timer_cancel(s_frame_timer);
    Pokemon_handle_focus_lost(s_graphics);
  } else {
    s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);
  
  s_background_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), s_background_layer);
  layer_mark_dirty(s_background_layer);

  s_graphics = GBC_Graphics_ctor(window);
  load_game();
  
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL);
}

static void window_unload(Window *window) {
  GBC_Graphics_destroy(s_graphics);

  unload_game();

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