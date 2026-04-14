#include "key.h"

static key_fun cb;
void key_init(key_desc_t key)
{
    GPIO_InitTypeDef GPIO_InitTStruct;
    GPIO_StructInit(&GPIO_InitTStruct);
    GPIO_InitTStruct.GPIO_Mode =  GPIO_Mode_IN;
    GPIO_InitTStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitTStruct.GPIO_Pin = key->pin;
    GPIO_Init(key->GPIO,&GPIO_InitTStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_Line0);
    
    EXTI_InitTypeDef EXTI_InitTStruct; //中断配置
    EXTI_InitTStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitTStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitTStruct.EXTI_Trigger= EXTI_Trigger_Falling;
    EXTI_InitTStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitTStruct);
    


    NVIC_InitTypeDef NVIC_InitTStruct;
    NVIC_InitTStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitTStruct.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitTStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitTStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitTStruct);

}
static struct key_desc _key1 ={GPIOA,GPIO_Pin_0};
key_desc_t key1 = &_key1;

void key_cb_register(key_fun key_cb)
{
    cb = key_cb;
}
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        if(cb)
            cb();
    }
    EXTI_ClearITPendingBit(EXTI_Line0);
}