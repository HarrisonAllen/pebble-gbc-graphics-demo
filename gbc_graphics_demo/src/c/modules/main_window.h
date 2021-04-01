#pragma once

#include <pebble.h>

#define FRAME_DURATION 2 // In milliseconds
#define FPS_TIMER_DELAY 1000 // ms

void main_window_mark_background_dirty();

void main_window_push();