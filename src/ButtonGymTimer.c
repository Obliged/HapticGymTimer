#include <pebble.h>
#include <pebble_fonts.h>
#define MAIN_FILE
#include "global.h" 
#include "WindowCtrl.h"

void uint16_to_time(uint16_t seconds, char* timer_str) {
  snprintf(timer_str, 6, "%02u:%02u", seconds/60 , seconds%60);
}

//Not static, global.
void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(mode < NUMBER_OF_MODES) mode++;
  else mode = 0;
  //Switch window. Push the new and remove the old.
  Window* top_window = window_stack_get_top_window();
 switch (mode) { 
  case 0 : 
    gym_timer_init(); 
    window_destroy(top_window);
    break;
  // case 1: 
  //  interval_init(); 
  //  window_destroy(top_window);
  //  break;
  default:
    do {
      window_destroy(top_window);
      top_window = window_stack_get_top_window();
    }while (top_window != NULL);
    gym_timer_init();
  }
}

static void init(void) {
  gym_timer_init();
}

static void deinit(void) {
  Window* top_window = window_stack_get_top_window();
  switch (mode) {
  case 0: 
    gym_timer_init();
    window_destroy(top_window);
    break;
  //case 1: 
  //  interval_init();
  //  window_destroy(top_window);
  //  break;
  default:
   do {
    window_destroy(top_window);
    top_window = window_stack_get_top_window();
   } while (top_window != NULL);
   gym_timer_init();
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
