#include <pebble.h>
#include "modules/main_window.h"

static void init(void) {
  main_window_push();
}

static void deinit(void) {
}

int main(void) {
  init();
  app_event_loop();
  APP_LOG(APP_LOG_LEVEL_WARNING, "Main deinit");
  deinit();
  APP_LOG(APP_LOG_LEVEL_WARNING, "Main exiting");
}
