#include <pebble.h>
#include "setup_window.h"
#include "run_window.h"

#define NUM_WINDOWS 3

static Window *s_interval_menu_window;
static MenuLayer *s_menu_layer;
static uint16_t stored_run_timer;
static uint16_t stored_pause_timer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_WINDOWS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Start", NULL, NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Running time", NULL, NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Pause time", NULL, NULL);
      break;
    default:
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    MENU_CELL_HEIGHT);
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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "menuWindowLoad:window: %p", window);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "menuWindowLoad:window_layer: %p", window_layer);
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, BGCOLOR, FGCOLOR);
  menu_layer_set_highlight_colors(s_menu_layer, FGCOLOR, BGCOLOR);
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
  persist_write_int(MEM_STORED_RUN_TIMER, stored_run_timer);
  persist_write_int(MEM_STORED_PAUSE_TIMER, stored_pause_timer);
  
  menu_layer_destroy(s_menu_layer);
  window_destroy(window);
  
  //s_interval_menu_window = NULL;
}

void interval_timer_init(void) {
  //Load previous counter
  stored_run_timer = (uint16_t) persist_read_int(MEM_STORED_RUN_TIMER);
  stored_pause_timer = (uint16_t) persist_read_int(MEM_STORED_PAUSE_TIMER);
  //Initialize if no value stored
  if (stored_run_timer == 0) stored_run_timer = 240;
  if (stored_pause_timer == 0) stored_pause_timer = 120;

  s_interval_menu_window = window_create();
  //window_set_click_config_provider(s_interval_menu_window, click_config_provider);
  window_set_background_color(s_interval_menu_window, PBL_IF_COLOR_ELSE(BGCOLOR, GColorWhite));
  window_set_window_handlers(s_interval_menu_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_interval_menu_window, true);
}