#include <stdbool.h>
#include <stdint.h>
#include "led.h"
#include "board.h"  
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "usart.h"
#include "key.h"
#include <string.h>
#include "queue.h"
#define BOOT 1
static volatile uint8_t rxbuf;
#define APP_ADDRESS        0x8010000UL
#define APP_FLASH_END      0x80FFFFFUL
#define SRAM_START         0x20000000UL
#define SRAM_SIZE          (128U * 1024U)      // STM32F407 常见 SRAM 128KB
#define SRAM_END           (SRAM_START + SRAM_SIZE)

typedef void (*pFunction)(void);

static uint8_t Boot_IsAppValid(uint32_t app_addr);
static void Boot_JumpToApp(uint32_t app_addr);
static void Boot_DeInit(void);
static void Error_Loop(void);

static void Delay(volatile uint32_t count)
{
    while (count--)
    {
        __NOP();
    }
}
static uint8_t Boot_ShouldEnterUpdate(void)
{
    if(BOOT == 1)
    {
        return 1U;
    }
    return 0U;
}
static void Boot_UpdateMode(void)
{
    uint32_t cnt = 0;
    while (cnt < 10)
    {
       led_on(led1);
       Delay(800000);
       led_off(led1);
       Delay(800000);
       cnt++;
    }
}


int main(void)
{
        
    board_init();
    led_Init(led1);
    led_Init(led2);
     
     
    if(Boot_ShouldEnterUpdate() != 0)
    {
        printf("Entering Update Mode...\r\n");
        Boot_UpdateMode();
    }

    if (Boot_IsAppValid(APP_ADDRESS))
    {
        printf("APP valid, jumping...\r\n");
        led_on(led2);
        
        Boot_JumpToApp(APP_ADDRESS);
    }
    Boot_UpdateMode();
    while (1)
    {
        Error_Loop();
        
    }

}


static uint8_t Boot_IsAppValid(uint32_t app_addr)
{
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4U);
    uint32_t app_reset_addr = app_reset & 0xFFFFFFFEU;
    if(app_stack < SRAM_START || app_stack > SRAM_END)
    {
        return 0U;
    }
    
    if ((app_reset_addr < APP_ADDRESS) || (app_reset_addr > APP_FLASH_END))
    {
        return 0U;
    }
    return 1U;
}
static void Boot_JumpToApp(uint32_t app_addr)
{
    uint32_t app_stack = *(volatile uint32_t *)app_addr;
    uint32_t app_reset = *(volatile uint32_t *)(app_addr + 4U);
    pFunction app_entry = (pFunction)app_reset;
    Boot_DeInit();

    SCB->VTOR = app_addr;

    __set_MSP(app_stack);
    __DSB();
    __ISB();

    app_entry ();
    while(1);
}
static void Boot_DeInit(void)
{
    __disable_irq();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    for(int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }
    __DSB();
    __ISB();

}
static void Error_Loop(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    while(1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        for(volatile int i = 0; i < 1000000; i++);
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        for(volatile int i = 0; i < 1000000; i++);
    }
}

void vAssertCalled(const char *file, int line)
{
    portDISABLE_INTERRUPTS();
    printf("Assert Called: %s(%d)\n", file, line);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack Overflowed: %s\n", pcTaskName);
    configASSERT(0);
}

void vApplicationMallocFailedHook(void)
{
    printf("Malloc Failed\n");
    configASSERT(0);
}
