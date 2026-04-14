#include "uos_task.h"
#include "stm32f4xx.h"
#include "board.h"
#include "led.h"
#include  <stdio.h>
#include "uos_list.h"

static TCB_T task1_tcb;
static TCB_T task2_tcb;
static uint32_t led_stack[128];
static uint32_t led_stack1[128];


static void busy_delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm volatile ("nop");
    }
}

void task1(void *arg)
{
    led_Init(led1);
    while(1)
    {
        led_on(led1);
        busy_delay(8000000);
        led_off(led1);
        busy_delay(8000000);

    }
}
void task2(void *arg)
{
    led_Init(led3);
    while(1)
    {
        led_on(led3);
        busy_delay(8000000);
        led_off(led3);
        busy_delay(8000000);

    }
}
int main(void)
{
    board_init();
    // led_Init(led3);

    os_task_create(&task1_tcb,task1,0,led_stack,128,1);
    os_task_create(&task2_tcb,task2,0,led_stack1,128,1);
    os_start(); // 启动调度器
    while(1);

}
