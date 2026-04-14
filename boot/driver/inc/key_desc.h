#ifndef _KEY_DESC_H
#define _KEY_DESC_H
#include "stm32f4xx.h"

struct key_desc
{
    GPIO_TypeDef *GPIO;
    uint16_t pin;
};

#endif // !_KEY_DESC_H
