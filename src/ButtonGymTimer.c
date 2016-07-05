#include <pebble.h>
#include <pebble_fonts.h>
#define MAIN_FILE
#include "global.h"

#define STORED_GYM_TIMER 0x00000000
#define GYM_TIMER_SECONDS 0x00000001
#define BGCOLOR GColorOrange
#define FONT FONT_KEY_LECO_36_BOLD_NUMBERS
#define TIMER_INTERVAL 30
#define CIRCLE_SIZE 120

static Window *window;
static TextLayer *text_layer;
static Layer *s_canvas_layer;
static bool timer_running;
static AppTimer * AppTimer_countdown;
static uint16_t stored_gym_timer;

static void uint16_to_time(uint16_t seconds, char* timer_str) {
  snprintf(timer_str, 6, "%02u:%02u", seconds/60 , seconds%60);
}

static void display_timer_time(void) {
  static char timer_str[6];
  uint16_to_time(gym_timer_seconds, timer_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
  text_layer_set_text(text_layer, timer_str);
}

static void countdown_callback(void) {
  if (gym_timer_seconds) {
    gym_timer_seconds--;
    //display_timer_time();
    AppTimer_countdown = app_timer_register(1000, (AppTimerCallback) countdown_callback, NULL);
  } else {
    vibes_short_pulse();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibrating!");
    timer_running = 0;
    gym_timer_seconds = stored_gym_timer;
    display_timer_time();  
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Stop
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;
  }
  //Start
  else {
    timer_running = 1;
    countdown_callback();
  }
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;
  }
  gym_timer_seconds = stored_gym_timer;
  display_timer_time();
}

static void up_repeating_click_handler(ClickRecognizerRef recognizer, void *context) {
  gym_timer_seconds += TIMER_INTERVAL;
  if (!timer_running) {
    stored_gym_timer += TIMER_INTERVAL;
    display_timer_time();
  } 
}

static void down_repeating_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(gym_timer_seconds - TIMER_INTERVAL > 0) gym_timer_seconds -= TIMER_INTERVAL;
  else gym_timer_seconds = 0;
  if (!timer_running) {
    if(stored_gym_timer - TIMER_INTERVAL > 0) stored_gym_timer -= TIMER_INTERVAL;
    else stored_gym_timer = 0;
    display_timer_time();
  }
}

static void click_config_provider(void *context) {
  uint16_t repeat_interval_ms = 400;
  uint16_t long_interval_ms = 700;
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);  
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_repeating_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_repeating_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, long_interval_ms, NULL, select_long_click_release_handler);
}

static void image_update_proc(Layer *layer, GContext *ctx) {
  // Place image in the center of the Window
  //GSize img_size = gdraw_command_image_get_bounds_size(s_command_image);
  GRect bounds = layer_get_bounds(layer);
  GRect cake_bounds = GRect(bounds.size.w/2-CIRCLE_SIZE/2, CIRCLE_SIZE, CIRCLE_SIZE, CIRCLE_SIZE);  
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 5);
  // Set the fill color
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  uint16_t inset_thickness = CIRCLE_SIZE/2;
  int32_t angle_start = DEG_TO_TRIGANGLE(360-360*gym_timer_seconds/stored_gym_timer);
  int32_t angle_end = DEG_TO_TRIGANGLE(360);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Angle end: %d", (int)angle_end);
  // Fill a radial section of a circle
  graphics_fill_radial(ctx, cake_bounds, GOvalScaleModeFitCircle, inset_thickness, angle_start, angle_end);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Clock
  text_layer = text_layer_create(GRect(0, bounds.size.h/2-46/2/*+45*/, bounds.size.w, 46));
  text_layer_set_background_color(text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, GColorBlack));
  text_layer_set_text_color(text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  //Image
  // Load the image and check it was succcessful
  //s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_DRAW_COMMAND);
  //if (!s_command_image) {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "Image is NULL!");
  //}

  // Create canvas Layer and set up the update procedure
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, image_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_background_color(window, PBL_IF_COLOR_ELSE(BGCOLOR, GColorWhite));
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  //Store counter
  (void) persist_write_int(STORED_GYM_TIMER, (uint32_t)stored_gym_timer);
  (void) persist_write_int(GYM_TIMER_SECONDS, (uint32_t)gym_timer_seconds);  
}

int main(void) {
  //Load previous counter
  stored_gym_timer = (uint16_t) persist_read_int(STORED_GYM_TIMER);
  gym_timer_seconds = (uint16_t) persist_read_int(GYM_TIMER_SECONDS);  
  
  //Initialize if no value stored
  if (stored_gym_timer == 0) stored_gym_timer = 60;
  if (gym_timer_seconds == 0) gym_timer_seconds = stored_gym_timer;
  
  timer_running = 0;
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}