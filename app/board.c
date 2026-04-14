
#include "board.h"
#include "led_desc.h"
#include <stdio.h>
#include "usart.h"
static struct led_desc _led1 = {
    .GPIO = GPIOB,
    .pin = GPIO_Pin_0,
    .action = 0,
};

static struct led_desc _led2 = {
    .GPIO = GPIOB,
    .pin = GPIO_Pin_1,
    .action = 1,
};
static struct led_desc _led3 = {
    .GPIO = GPIOE,
    .pin = GPIO_Pin_9,
    .action = 0,
};
led_t led1 = &_led1;//data
led_t led2 = &_led2;
led_t led3 = &_led3;

void board_init(void)
{


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART2EN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 


}
