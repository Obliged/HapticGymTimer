#include <pebble.h>
#include "interval_timer.h"
#include "setup_window.h"
#include "run_window.h"

#define NUM_WINDOWS 3

static Window *s_interval_menu_window;
static MenuLayer *s_menu_layer;
static uint16_t stored_run_timer;
static uint16_t stored_pause_timer;
static uint16_t curr_timer; //FIXME move to run
static bool pause; //FIXME move to run

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_WINDOWS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Run Timer", NULL, NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Setup running time", NULL, NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Setup pause time", NULL, NULL);
      break;
    default:
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    CHECKBOX_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      interval_run_window_push(&stored_run_timer, &stored_pause_timer);
      break;
    case 1:
      interval_setup_window_push(&stored_run_timer, RUN_SETUP);
      break;
    case 2:
      interval_setup_window_push(&stored_pause_timer, PAUSE_SETUP);
      break;
    default:
      break;
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorOrange, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorWhite, GColorOrange);
#endif
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void interval_timer_init() {
    //Load previous counter
  stored_run_timer = (uint16_t) persist_read_int(MEM_STORED_RUN_TIMER);
  stored_pause_timer = (uint16_t) persist_read_int(MEM_STORED_PAUSE_TIMER);
  curr_timer = (uint16_t) persist_read_int(MEM_CURR_TIMER); //FIXME move to run
  pause = (bool) persist_read_bool(MEM_PAUSE); //FIXME move to run
  //Initialize if no value stored
  if (stored_run_timer == 0) stored_run_timer = 120;
  if (stored_pause_timer == 0) stored_pause_timer = 120;
  if (curr_timer == 0) curr_timer = pause ? stored_pause_timer : stored_run_timer;
  timer_running = 0;
  
  s_interval_menu_window = window_create();
  //window_set_click_config_provider(s_interval_menu_window, click_config_provider);
  window_set_background_color(s_interval_menu_window, PBL_IF_COLOR_ELSE(BGCOLOR, GColorWhite));
  window_set_window_handlers(s_interval_menu_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_interval_menu_window, true);
}

static void interval_timer_deinit() {
  window_destroy(s_interval_menu_window);
}