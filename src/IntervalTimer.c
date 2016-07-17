#include <pebble.h>
#include <pebble_fonts.h>
#include "global.h"

static Window *interval_timer_window;
static TextLayer *run_text_layer;
static TextLayer *pause_text_layer;
static TextLayer *setup_text_layer;
static Layer *s_canvas_layer;
static ActionMenuLevel* interval_menu;
static bool timer_running;
static AppTimer * AppTimer_countdown;
static uint16_t stored_run_timer;
static uint16_t stored_pause_timer;
static uint16_t curr_timer;
static bool running;

static void display_timer_time(void) {
  char timer_str[6];
  uint16_to_time(curr_timer, timer_str);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds_str now: %s", timer_str);
  if( pause ) {
    text_layer_set_text(pause_text_layer, timer_str);
  } else {
    text_layer_set_text(run_text_layer, timer_str);    
  }
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
      curr_timer = stored_pause_timer;
      uint16_to_time(stored_run_timer, timer_str);
      text_layer_set_texy(run_text_layer, timer_str);
    } else {
      curr_timer = stored_run_timer;
      uint16_to_time(stored_pause_timer, timer_str);
      text_layer_set_texy(pause_text_layer, timer_str);      
    }
  }
  AppTimer_countdown = app_timer_register(1000, (AppTimerCallback) countdown_callback, NULL);
  display_timer_time();  
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Stop
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;  
    display_timer_time();
    layer_set_hidden(s_canvas_layer, true);
  }
  //Start
  else {
    timer_running = 1;
    countdown_callback();
    layer_set_hidden(s_canvas_layer, false);
  }
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    app_timer_cancel(AppTimer_countdown);
    timer_running = 0;
  }
  curr_timer = stored_run_timer;
  display_timer_time();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if( timer_running ) {
    curr_timer += TIMER_INTERVAL;
  } else {
    stored_run_timer += TIMER_INTERVAL;
    display_timer_time();
  } 
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if ( timer_running ){
    if(curr_timer - TIMER_INTERVAL > 0) curr_timer -= TIMER_INTERVAL;
    else curr_timer = 0;
  } else {
    if(stored_run_timer - TIMER_INTERVAL > 0) stored_run_timer -= TIMER_INTERVAL;
    else stored_run_timer = 0;
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
  GRect cake_bounds = GRect(bounds.size.w/2-CIRCLE_SIZE/2, CIRCLE_SIZE, CIRCLE_SIZE, CIRCLE_SIZE);  
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 5);
  // Set the fill color
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  uint16_t inset_thickness = CIRCLE_SIZE/2;
  int32_t angle_start = DEG_TO_TRIGANGLE(360-360*curr_timer/(pause ? stored_pause_timer : stored_run_timer));
  int32_t angle_end = DEG_TO_TRIGANGLE(360);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Angle end: %d", (int)angle_end);
  // Fill a radial section of a circle
  graphics_fill_radial(ctx, cake_bounds, GOvalScaleModeFitCircle, inset_thickness, angle_start, angle_end);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Run text
  run_text_layer = text_layer_create(GRect(0, bounds.size.h/2-46/2-46, bounds.size.w, 46));
  text_layer_set_background_color(run_text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, GColorBlack));
  text_layer_set_text_color(run_text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(run_text_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(run_text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(run_text_layer));
  
  //Pause text
  pause_text_layer = text_layer_create(GRect(0, bounds.size.h/2-46/2+46, bounds.size.w, 46));
  text_layer_set_background_color(pause_text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, GColorBlack));
  text_layer_set_text_color(pause_text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(pause_text_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(pause_text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(pause_text_layer));
  
  // Create canvas Layer and set up the update procedure
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, image_update_proc);
  layer_set_hidden(s_canvas_layer, true);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(run_text_layer);
  text_layer_destroy(pause_text_layer);
  layer_destroy(s_canvas_layer);

  (void) persist_write_int(MEM_STORED_RUN_TIMER, (uint32_t)stored_gym_timer);
  (void) persist_write_int(MEM_STORED_PAUSE_TIMER, (uint32_t)stored_pause_timer);  
  (void) persist_write_int(MEM_STORED_CURR_TIMER, (uint32_t)stored_curr_timer);
  (void) persist_write_bool(MEM_PAUSE, pause);
}

static void setup_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Setup text
  setup_text_layer = text_layer_create(GRect(0, bounds.size.h/2-46/2, bounds.size.w, 46));
  text_layer_set_background_color(setup_text_layer, PBL_IF_COLOR_ELSE(BGCOLOR, GColorBlack));
  text_layer_set_text_color(setup_text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(setup_text_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(setup_text_layer, GTextAlignmentCenter);
  display_timer_time();
  layer_add_child(window_layer, text_layer_get_layer(setup_text_layer));
  }

static void setup_window_unload(Window *window) {
  text_layer_destroy(setp_text_layer);
}

void interval_timer_init(void) {
  //Load previous counter
  stored_run_timer = (uint16_t) persist_read_int(MEM_STORED_RUN_TIMER);
  stored_pause_timer = (uint16_t) persist_read_int(MEM_STORED_PAUSE_TIMER);
  curr_timer = (uint16_t) persist_read_int(MEM_CURR_TIMER);
  pause = (bool) persist_read_bool(MEM_PAUSE);
  //Initialize if no value stored
  if (stored_run_timer == 0) stored_run_timer = 120;
  if (stored_pause_timer == 0) stored_pause_timer = 120;
  if (curr_timer == 0) curr_timer = pause ? stored_pause_timer : stored_run_timer;
  timer_running = 0;
  
  interval_timer_window = window_create();
  window_set_click_config_provider(interval_timer_window, click_config_provider);
  window_set_background_color(interval_timer_window, PBL_IF_COLOR_ELSE(BGCOLOR, GColorWhite));
  window_set_window_handlers(interval_timer_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(interval_timer_window, false);

  interval_menu = action_menu_level_create(3);
  action_menu_level_set_display_mode(interval_menu, ActionMenuLevelDisplayModeWide);
  start_item = action_menu_level_add_action(interval_menu, "Start" , start_interval_timer, NULL);
  run_item = action_menu_level_add_action(interval_menu, "Interval time" , setup_timer, &stored_run_timer);
  pause_item = action_menu_level_add_action(interval_menu, "Pause time" , setup_timer, &stored_pause_timer);
}

static void setup_timer(uint16_t* stored_timer) {
  setup_timer_window = window_create();
  window_set_click_config_provider(setup_timer_window, setup_click_config_provider);
  window_set_background_color(setup_timer_window, PBL_IF_COLOR_ELSE(BGCOLOR, GColorWhite));
  window_set_window_handlers(setup_timer_window, (WindowHandlers) {
    .load = setup_window_load,
    .unload = setup_window_unload,
  });
  window_stack_push(setup_timer_window, false);
}

void interval_timer_deinit(void) {
  window_destroy(interval_timer_window);
}
