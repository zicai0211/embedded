#include "led.h"

void led_Init(led_t led)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = led->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(led->GPIO, &GPIO_InitStructure);
    
}
void led_on(led_t led)
{
     GPIO_WriteBit(led->GPIO, led->pin, Bit_RESET);
}
void led_off(led_t led)
{
    GPIO_WriteBit(led->GPIO, led->pin, Bit_SET);
}
void led_set(led_t led, bool state)
{
    GPIO_WriteBit(led->GPIO, led->pin, state ? Bit_RESET : Bit_SET);
}
