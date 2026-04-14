#include "usart.h"

static volatile char rx_data;


SemaphoreHandle_t usart_done_semaphore ;
SemaphoreHandle_t usart_busy_semaphore ;

static usart_send_cb cb;
static void gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

}
static void usart(){

    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 115200u;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;

    USART_Init(USART1, &USART_InitStructure);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

    USART_Cmd(USART1, ENABLE);
}
static void nvic_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitTypeDef NVIC_InitTStruct;
    NVIC_InitTStruct.NVIC_IRQChannel = DMA2_Stream7_IRQn;
    NVIC_InitTStruct.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitTStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitTStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitTStruct);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;//DMA2 Stream7 Channel4
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;//直接写入dr寄存器
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;//内存地址，后续会更新
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//数据传输方向：内存到外设
    DMA_InitStructure.DMA_BufferSize = 0;//数据传输量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度：字节
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据宽度：字节
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//工作在正常模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//优先级为中
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;//不使用FIFO
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//内存单次传输
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
    // DMA_Cmd(DMA2_Stream7, ENABLE);
}
void usart_init()
{
    gpio_init();
    usart();
    
    
    usart_done_semaphore = xSemaphoreCreateBinary();
    configASSERT(usart_done_semaphore);

    usart_busy_semaphore = xSemaphoreCreateBinary();
    configASSERT(usart_busy_semaphore);
    xSemaphoreGive(usart_busy_semaphore); 
    nvic_init();

    

}
 void usart_send_dma(uint8_t *data, uint16_t size)
{
    xSemaphoreTake(usart_busy_semaphore, portMAX_DELAY);
    DMA_Cmd(DMA2_Stream7, DISABLE);
    while (DMA2_Stream7->CR & DMA_SxCR_EN); 
    DMA2_Stream7->M0AR = (uint32_t)data;//传输数据
    DMA2_Stream7->NDTR = size;//设置传输数据量
    

    DMA_Cmd(DMA2_Stream7, ENABLE);
    xSemaphoreTake(usart_done_semaphore, portMAX_DELAY);

    xSemaphoreGive(usart_busy_semaphore);
}

void send_data(uint8_t data)
{
    USART_SendData(USART1, data);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
}
void usart_send_register(usart_send_cb call_back)
{
    cb = call_back;
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t res = USART_ReceiveData(USART1);
        if(cb)
            cb(res);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        
    }
}
// void DMA2_Stream7_IRQHandler(void)
// {
//     if(DMA_GetFlagStatus(DMA2_Stream7,DMA_FLAG_TCIF7) !=  RESET)
//     {
//         DMA_ClearFlag(DMA2_Stream7,DMA_FLAG_TCIF7);
//         DMA_Cmd(DMA2_Stream7, DISABLE);
//         BaseType_t pxHigherPriorityTaskWoken  = pdFALSE;
//         xSemaphoreGiveFromISR(usart_done_semaphore, &pxHigherPriorityTaskWoken);
//         portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
//     }
// }