#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h"
#include "led_desc.h"
struct led_desc;
typedef struct led_desc *led_t;

void led_Init(led_t led);
void led_on(led_t led);
void led_off(led_t led);
void led_set(led_t led, bool state);

#endif
