#pragma once

#include <pebble.h>

#if defined(PBL_COLOR)
#define FRAME_DURATION 2 // In milliseconds
#else
#define FRAME_DURATION 2
#endif
#define FPS_TIMER_DELAY 1000 // ms

void main_window_mark_background_dirty();

void main_window_push();