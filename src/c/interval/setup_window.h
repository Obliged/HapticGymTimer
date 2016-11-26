#pragma once

#include <pebble.h>
#include <pebble_fonts.h>
#include "../global.h"

#define RUN_SETUP 1
#define PAUSE_SETUP 0

void interval_setup_window_push(uint16_t* stored_timer, bool run_setup);