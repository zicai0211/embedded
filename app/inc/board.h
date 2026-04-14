#ifndef __BOARD_H
#define __BOARD_H
#include "led.h"

extern led_t led1; 
extern led_t led2; 
extern led_t led3; 

void board_init(void);
#endif // !__BOARD_H
