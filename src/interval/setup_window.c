#include "setup_window.h"

static Window* s_main_window;
static TextLayer* s_text_layer;
uint16_t* local_timer;

static void display_timer_time(void) {
  char timer_str[6];
  uint16_to_time(*local_timer, timer_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
  text_layer_set_text(s_text_layer, timer_str);    
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    *local_timer += TIMER_INTERVAL;
    display_timer_time();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if(*local_timer - TIMER_INTERVAL > 0) *local_timer -= TIMER_INTERVAL;
    else *local_timer = 0;
    display_timer_time();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void window_load(){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Run text
  s_text_layer = text_layer_create(GRect(0, bounds.size.h/2-TEXT_LAYER_H/2, bounds.size.w, TEXT_LAYER_H));
  text_layer_set_background_color(s_text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, GColorBlack));
  text_layer_set_text_color(s_text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  window_destroy(window);
  s_main_window = NULL;
}

void interval_setup_window_push(uint16_t* stored_timer, bool run_setup){
  local_timer = stored_timer;
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}