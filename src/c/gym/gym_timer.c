#include <pebble.h>
#include <pebble_fonts.h>
#include "global.h"

static Window *gym_timer_window;
static TextLayer *text_layer;
static Layer *s_canvas_layer;
static bool timer_running;
static AppTimer *AppTimer_countdown;
static uint16_t stored_gym_timer;
static uint16_t gym_timer;

static void display_timer_time(void) {
  static char timer_str[6];
  uint16_to_time(gym_timer, timer_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
  text_layer_set_text(text_layer, timer_str);
}

static void countdown_callback(void) {
  if (gym_timer) {
    gym_timer--;
    layer_mark_dirty(s_canvas_layer);
    AppTimer_countdown = app_timer_register(1000, (AppTimerCallback) countdown_callback, NULL);
  } else {
    vibes_short_pulse();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrating!");
    timer_running = 0;
    gym_timer = stored_gym_timer;
    display_timer_time();  
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Stop
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;  
    display_timer_time();
    layer_set_hidden(s_canvas_layer, true);
    layer_set_hidden((Layer*)text_layer, false);
  }
  //Start
  else {
    timer_running = 1;
    countdown_callback();
    layer_set_hidden(s_canvas_layer, false);
    layer_set_hidden((Layer*)text_layer, true);
  }
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;
  }
  gym_timer = stored_gym_timer;
  display_timer_time();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  gym_timer += TIMER_INTERVAL;
  if (!timer_running) {
    stored_gym_timer += TIMER_INTERVAL;
    display_timer_time();
  } 
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(gym_timer - TIMER_INTERVAL > 0) gym_timer -= TIMER_INTERVAL;
  else gym_timer = 0;
  if (!timer_running) {
    if(stored_gym_timer - TIMER_INTERVAL > 0) stored_gym_timer -= TIMER_INTERVAL;
    else stored_gym_timer = 0;
    display_timer_time();
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);  
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, LONG_INTERVAL, NULL, select_long_click_release_handler);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 2, MULTI_INTERVAL, false, select_multi_click_handler);
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
  int32_t angle_start = DEG_TO_TRIGANGLE(360-360*gym_timer/stored_gym_timer);
  int32_t angle_end = DEG_TO_TRIGANGLE(360);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Angle start: %d", (int)angle_start);
  // Fill a radial section of a circle
  graphics_fill_radial(ctx, cake_bounds, GOvalScaleModeFitCircle, inset_thickness, angle_start, angle_end);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Clock
  text_layer = text_layer_create(GRect(0, bounds.size.h/2-TEXT_LAYER_H/2, bounds.size.w, TEXT_LAYER_H));
  text_layer_set_background_color(text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, BWBGCOLOR));
  text_layer_set_text_color(text_layer, PBL_IF_COLOR_ELSE(FGCOLOR, BWFGCOLOR));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  // Create canvas Layer and set up the update procedure
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, image_update_proc);
  layer_set_hidden(s_canvas_layer, true);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  if(timer_running) app_timer_cancel(AppTimer_countdown);
  text_layer_destroy(text_layer);
  layer_destroy(s_canvas_layer);
  
  (void) persist_write_int(MEM_STORED_GYM_TIMER, (uint32_t)stored_gym_timer);
  (void) persist_write_int(MEM_GYM_TIMER, (uint32_t)gym_timer);  
  window_destroy(window);
}

void gym_timer_init(void) {
  //Load previous counter
  stored_gym_timer = (uint16_t) persist_read_int(MEM_STORED_GYM_TIMER);
  gym_timer = (uint16_t) persist_read_int(MEM_GYM_TIMER);  
  //Initialize if no value stored
  if (stored_gym_timer == 0) stored_gym_timer = 60;
  if (gym_timer == 0) gym_timer = stored_gym_timer;
  timer_running = 0;
  
  gym_timer_window = window_create();
  window_set_click_config_provider(gym_timer_window, click_config_provider);
  window_set_background_color(gym_timer_window, PBL_IF_COLOR_ELSE(BGCOLOR, BWBGCOLOR));
  window_set_window_handlers(gym_timer_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(gym_timer_window, animated);
}

