#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"
#include <stdint.h>
#include "stm32f4xx_usart.h"
#include "FreeRTOS.h"
#include "semphr.h"



typedef void (*usart_send_cb)(uint8_t data);


void usart_init(void);
void send_data(uint8_t data);
void usart_send_register(usart_send_cb call_back);
void usart_send_dma(uint8_t *data, uint16_t size);

extern SemaphoreHandle_t usart_done_semaphore;
extern SemaphoreHandle_t usart_busy_semaphore;
#endif // !__USART_H