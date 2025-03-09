#pragma once

#include <pebble.h>

// Very low frame duration, draw as fast as possible. 1ms leads
// to strange behavior, so I alwyas go with 2
#define FRAME_DURATION 2

void main_window_mark_background_dirty();

void main_window_push();