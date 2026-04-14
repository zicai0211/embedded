#ifndef __LED_DESC_
#define __LED_DESC_
#include <stdint.h>
#include "stm32f4xx.h"
#include <stdbool.h>
struct led_desc {
    GPIO_TypeDef *GPIO ;
    uint16_t pin;
    bool action;
};

#endif // DEBUG