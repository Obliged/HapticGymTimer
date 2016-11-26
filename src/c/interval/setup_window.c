#include "setup_window.h"

static Window* s_main_window;
static TextLayer* s_text_layer;
uint16_t local_timer;
uint16_t* stored_timer_handle;
static AppTimer *AppTimer_countdown;

static void display_timer_time(void) {
  static char timer_str[6];
  uint16_to_time(local_timer, timer_str);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_text_layer: %p", s_text_layer);
  text_layer_set_text(s_text_layer, timer_str);    
}

static void exit_callback(){
    window_stack_pop(true);  
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    local_timer += TIMER_INTERVAL;
    display_timer_time();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if(local_timer - TIMER_INTERVAL > 0) local_timer -= TIMER_INTERVAL;
    else local_timer = 0;
    display_timer_time();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  static char timer_str[6];
  *stored_timer_handle = local_timer;
  snprintf(timer_str, 6, "Saved");
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_text_layer, timer_str);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_text_layer: %p", s_text_layer);
  AppTimer_countdown = app_timer_register(1000, (AppTimerCallback) exit_callback, NULL);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 2, MULTI_INTERVAL, false, select_multi_click_handler);
}

static void window_load(Window* window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WindowLoad:window: %p", window);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WindowLoad:window_layer: %p", window_layer);

  //Run text
  s_text_layer = text_layer_create(GRect(0, bounds.size.h/2-TEXT_LAYER_H/2, bounds.size.w, TEXT_LAYER_H));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WindowLoad:s_text_layer: %p", s_text_layer);
  text_layer_set_background_color(s_text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, BWBGCOLOR));
  text_layer_set_text_color(s_text_layer, PBL_IF_COLOR_ELSE(FGCOLOR, BWFGCOLOR));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  display_timer_time();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WindowLoad:s_text_layer text: %s", text_layer_get_text(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  window_destroy(window);
  s_main_window = NULL;
}

void interval_setup_window_push(uint16_t* stored_timer, bool run_setup){
  stored_timer_handle = stored_timer;
  local_timer = *stored_timer;
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(BGCOLOR, BWBGCOLOR));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}