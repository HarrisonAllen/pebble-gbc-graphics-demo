#pragma once

#include <pebble.h>
#include <pebble-gbc-graphics/pebble-gbc-graphics.h>

void Pokemon_initialize(GBC_Graphics *graphics);
void Pokemon_deinitialize(GBC_Graphics *graphics);
void Pokemon_step(GBC_Graphics *graphics);
void Pokemon_handle_select(GBC_Graphics *graphics, bool pressed);
void Pokemon_handle_down(GBC_Graphics *graphics);
void Pokemon_handle_tap(GBC_Graphics *graphics);
void Pokemon_handle_focus_lost(GBC_Graphics *graphics);