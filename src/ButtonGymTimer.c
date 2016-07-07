#include <pebble.h>
#include <pebble_fonts.h>
#define MAIN_FILE
#include "global.h" 
#include "WindowCtrl.h"

static Window *window;
static TextLayer *text_layer;
static Layer *s_canvas_layer;
static bool timer_running;
static AppTimer * AppTimer_countdown;
static uint16_t stored_gym_timer;

void uint16_to_time(uint16_t seconds, char* timer_str) {
  snprintf(timer_str, 6, "%02u:%02u", seconds/60 , seconds%60);
}

//Not static, global.
void select_multi_cick_handler(ClickRecognizerRef recognizer, void *context) {
  if(mode < NUMBEROFMODES) mode++;
  else mode = 0;
  //Switch window. Push the new and remove the old.
  Window* top_window = window_stack_get_top_window();
 case 
   0: {gym_timer_init(); window_destroy(top_window);};
 1: { interval_init(); window_destroy(top_window); }
 default:
   do {
     window_destroy(top_window);
     top_window = window_stack_get_top_window();
   }while (top_window != NULL);
   gym_timer_init();

static void init(void) {
  gym_timer_init();
}

static void deinit(void) {
    Window* top_window = window_stack_get_top_window();
 case 
   0: {gym_timer_init(); window_destroy(top_window);};
 1: { interval_init(); window_destroy(top_window); }
 default:
   do {
     window_destroy(top_window);
     top_window = window_stack_get_top_window();
   }while (top_window != NULL);
   gym_timer_init();

  window_destroy(window);  
}

int main(void) {
  //Load previous counter
  stored_gym_timer = (uint16_t) persist_read_int(MEM_STORED_GYM_TIMER);
  gym_timer = (uint16_t) persist_read_int(MEM_GYM_TIMER);  
  
  //Initialize if no value stored
  if (stored_gym_timer == 0) stored_gym_timer = 60;
  if (gym_timer == 0) gym_timer = stored_gym_timer;
  
  timer_running = 0;
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
