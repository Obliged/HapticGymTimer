#include "run_window.h"

static Window* s_main_window;
static TextLayer* s_run_layer;
static TextLayer* s_pause_layer;
static Layer* s_canvas_layer;
static AppTimer * AppTimer_countdown;
static uint16_t local_run_timer;
static uint16_t local_pause_timer;
static uint16_t curr_timer;
static bool timer_running;
static bool pause;

static void display_timer_time(void) {
  char timer_str[6];
  uint16_to_time(curr_timer, timer_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
  if( pause ) {
    text_layer_set_text(s_pause_layer, timer_str);
  } else {
    text_layer_set_text(s_run_layer, timer_str);    
  }
}

static void image_update_proc(Layer *layer, GContext *ctx) {
  // Place image in the center of the Window
  //GSize img_size = gdraw_command_image_get_bounds_size(s_command_image);
  GRect bounds = layer_get_bounds(layer);
  GRect cake_bounds = GRect(bounds.size.w/2-CIRCLE_SIZE/2, bounds.size.h/2-CIRCLE_SIZE/2, CIRCLE_SIZE, CIRCLE_SIZE);  
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 5);
  // Set the fill color
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(FGCOLOR, BWFGCOLOR));
  
  uint16_t inset_thickness = CIRCLE_SIZE/2;
  int32_t angle_start = DEG_TO_TRIGANGLE(360-360*curr_timer/(pause ? local_pause_timer : local_run_timer));
  int32_t angle_end = DEG_TO_TRIGANGLE(360);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Angle start: %d", (int)angle_start);
  // Fill a radial section of a circle
  graphics_fill_radial(ctx, cake_bounds, GOvalScaleModeFitCircle, inset_thickness, angle_start, angle_end);
}

static void countdown_callback(void) {
  if (curr_timer) {
    curr_timer--;
  } else {
    vibes_short_pulse();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrating!");
    pause = !pause;
    char timer_str[6];
    if ( pause ) {
      curr_timer = local_pause_timer;
      uint16_to_time(local_run_timer, timer_str);
      text_layer_set_text(s_run_layer, timer_str);
    } else {
      curr_timer = local_run_timer;
      uint16_to_time(local_pause_timer, timer_str);
      text_layer_set_text(s_pause_layer, timer_str);      
    }
  }
  AppTimer_countdown = app_timer_register(1000, (AppTimerCallback) countdown_callback, NULL);
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;
  }
  curr_timer = pause ? local_pause_timer : local_run_timer;
  display_timer_time();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Stop
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;  
    display_timer_time();
    layer_set_hidden((Layer*)s_pause_layer,false);
    layer_set_hidden((Layer*)s_run_layer,false);
    layer_set_hidden(s_canvas_layer, true);
  }
  //Start
  else {
    timer_running = 1;
    countdown_callback();
    layer_set_hidden((Layer*)s_pause_layer,true);
    layer_set_hidden((Layer*)s_run_layer,true);
    layer_set_hidden(s_canvas_layer, false);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, LONG_INTERVAL, NULL, select_long_click_release_handler);
}

static void window_load(Window* window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Run text
  s_run_layer = text_layer_create(GRect(0, bounds.size.h/2-TEXT_LAYER_H/2-TEXT_LAYER_H, bounds.size.w, TEXT_LAYER_H));
  text_layer_set_background_color(s_run_layer, PBL_IF_COLOR_ELSE(GColorBlack, BWBGCOLOR));
  text_layer_set_text_color(s_run_layer, PBL_IF_COLOR_ELSE(FGCOLOR, BWFGCOLOR));
  text_layer_set_font(s_run_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(s_run_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(s_run_layer));

  //Pause text
  s_pause_layer = text_layer_create(GRect(0, bounds.size.h/2-TEXT_LAYER_H/2+TEXT_LAYER_H, bounds.size.w, TEXT_LAYER_H));
  text_layer_set_background_color(s_pause_layer, PBL_IF_COLOR_ELSE(BGCOLOR, BWBGCOLOR));
  text_layer_set_text_color(s_pause_layer, PBL_IF_COLOR_ELSE(FGCOLOR, BWFGCOLOR));
  text_layer_set_font(s_pause_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(s_pause_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(s_pause_layer));

  // Create canvas Layer and set up the update procedure
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, image_update_proc);
  layer_set_hidden(s_canvas_layer, true);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  (void) persist_write_int(MEM_CURR_INTERVAL, (uint32_t)curr_timer);
  (void) persist_write_bool(MEM_PAUSE, pause);

  window_destroy(window);
  s_main_window = NULL;
}

void interval_run_window_push(uint16_t* stored_run_timer, uint16_t* stored_pause_timer){
  local_run_timer = *stored_run_timer;
  local_pause_timer = *stored_pause_timer;
  curr_timer = (uint16_t) persist_read_int(MEM_CURR_INTERVAL);
  pause = persist_read_bool(MEM_PAUSE);
  if (curr_timer == 0) {
    pause = 0;
    curr_timer = *stored_run_timer;
  }
  timer_running = 0;
    
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