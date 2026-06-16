#include <math.h>
#include "main_window.h"
#include "pebble-gbc-graphics/pebble-gbc-graphics.h"
#include "pokemon/pokemon.h"
#include "pokemon/enums.h"

static Window *s_window;
static GBC_Graphics *s_graphics;
static AppTimer *s_frame_timer;
static Layer *s_background_layer;
#if defined(PBL_PLATFORM_EMERY)
#define DPAD_W 111 // width
#define DPAD_H 111 // height
#define DPAD_X 0
#define DPAD_Y 130
#define DPAD_M_X (DPAD_X + 55) // middle x
#define DPAD_M_Y (DPAD_Y + 55) // middle y
static TouchButtonType s_current_button = TOUCH_NONE;

static const GRect touch_a_bounds = GRect(152, 180, 48, 48);
static const GRect touch_b_bounds = GRect(107, 137, 59, 56);
static const GRect touch_dpad_bounds = GRect(DPAD_X, DPAD_Y, DPAD_W, DPAD_H);
static const int32_t dpad_angles[4][2] = {
  {135, 225}, // up
  {225, 315}, // left
  {-1, -1}, // down is on 315 < a < 45 which is a special case, so just do if none of the above
  {45, 135}, // right
};
#endif

/* Game loading handlers */
static void load_game() {
  Pokemon_initialize(s_window, s_graphics, s_background_layer);
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

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_down(s_graphics);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  Pokemon_handle_back(s_graphics);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);

  window_raw_click_subscribe(BUTTON_ID_SELECT, select_press_handler, select_release_handler, NULL);
}

#if defined(PBL_PLATFORM_EMERY)
TouchButtonType get_button_from_touch_location(int16_t x, int16_t y) {
  GPoint touch_point = GPoint(x, y);
  // Check A
  if (grect_contains_point(&touch_a_bounds, &touch_point)) {
    return TOUCH_A;
  }

  // Check B
  if (grect_contains_point(&touch_b_bounds, &touch_point)) {
    return TOUCH_B;
  }

  // Check dpad
  if (grect_contains_point(&touch_dpad_bounds, &touch_point)) {
    int32_t angle = TRIGANGLE_TO_DEG(atan2_lookup(x - DPAD_M_X, y - DPAD_M_Y));

    uint8_t i;
    TouchButtonType default_direction = TOUCH_NONE;
    for (i = 0; i < 4; i++) {
      if (dpad_angles[i][0] == -1) {
        default_direction = TOUCH_UP + i;
      } else if (angle >= dpad_angles[i][0] && angle < dpad_angles[i][1]) {
        return TOUCH_UP + i;
      }
    }
    return default_direction;
  }

  return TOUCH_NONE;
}

static void handle_touchdown(int16_t x, int16_t y) {
  s_current_button = get_button_from_touch_location(x, y);

  switch (s_current_button) {
    case TOUCH_A:
      Pokemon_handle_press_a(s_graphics);
      break;
    case TOUCH_B:
      Pokemon_handle_press_b(s_graphics);
      break;
    case TOUCH_UP:
      Pokemon_handle_press_up(s_graphics);
      break;
    case TOUCH_LEFT:
      Pokemon_handle_press_left(s_graphics);
      break;
    case TOUCH_DOWN:
      Pokemon_handle_press_down(s_graphics);
      break;
    case TOUCH_RIGHT:
      Pokemon_handle_press_right(s_graphics);
      break;
    default:
      break;
  }
}

static void handle_liftoff(int16_t x, int16_t y) {
  switch (s_current_button) {
    case TOUCH_UP:
      Pokemon_handle_release_up(s_graphics);
      break;
    case TOUCH_LEFT:
      Pokemon_handle_release_left(s_graphics);
      break;
    case TOUCH_DOWN:
      Pokemon_handle_release_down(s_graphics);
      break;
    case TOUCH_RIGHT:
      Pokemon_handle_release_right(s_graphics);
      break;
    default:
      break;
  }

  s_current_button = TOUCH_NONE;
}

static void touch_handler(const TouchEvent *event, void *context) {
  switch (event->type) {
    case TouchEvent_Touchdown:
      handle_touchdown(event->x, event->y);
      break;
    case TouchEvent_PositionUpdate:
      break;
    case TouchEvent_Liftoff:
      handle_liftoff(event->x, event->y);
      break;
  }
}
#endif

static void frame_timer_handle(void* context) {
  // requeue timer at start, not sure if it's too fast for the logic and will break
  s_frame_timer = app_timer_register(FRAME_DURATION, frame_timer_handle, NULL); 

  // main game loop
  Pokemon_step(s_graphics);

  // Here, I draw every frame to achieve a consistent frame rate
  // However, it's possible to just draw when necessary for
  // better battery life and faster frame rates
  GBC_Graphics_render(s_graphics);

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
  
  s_background_layer = layer_create_with_data(
      bounds, 
      sizeof(s_graphics)
  );
  layer_add_child(window_get_root_layer(window), s_background_layer);
  layer_mark_dirty(s_background_layer);

  s_graphics = GBC_Graphics_ctor(window);
  *(GBC_Graphics * *)layer_get_data(s_background_layer) = s_graphics;
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
    app_focus_service_subscribe(will_focus_handler);
#if defined(PBL_PLATFORM_EMERY)
    touch_service_subscribe(touch_handler, NULL);
#endif
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}