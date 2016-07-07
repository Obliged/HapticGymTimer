
#ifndef GLOBAL_INCLUDED
#define GLOBAL_INCLUDED
#ifdef  MAIN_FILE
uint16_t gym_timer_seconds;
uint8_t mode;
#else
#define MEM_STORED_GYM_TIMER 0x00000000
#define MEM_GYM_TIMER 0x00000001

#define BGCOLOR GColorOrange
#define FONT FONT_KEY_LECO_36_BOLD_NUMBERS
#define CIRCLE_SIZE 120

#define TIMER_INTERVAL 30
#define REPEAT_INTERVAL 400;
#define LONG_INTERVAL 700;
#define MULTI_INTERVAL 300;

extern uint16_t gym_timer_seconds;
extern uint8_t mode;
void select_multi_cick_handler(ClickRecognizerRef recognizer, void *context);
void uint16_to_time(uint16_t seconds, char* timer_str);
#endif
#endif
