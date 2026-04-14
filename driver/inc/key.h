#ifndef _KEY_H_
#define _KEY_H_
#include "stm32f4xx.h"
#include "key_desc.h"
#include "stm32f4xx_gpio.h"
struct key_desc;

typedef struct key_desc *key_desc_t;

typedef void(*key_fun)(void);

void key_init(key_desc_t key);
void key_cb_register(key_fun key_cb);


extern  key_desc_t key1;
#endif // !_KEY_H_
