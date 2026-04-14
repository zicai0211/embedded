#ifndef UOS_TASK_H_
#define UOS_TASK_H_
#include <stdint.h>
#include "stm32f4xx.h"
#include "uos_list.h"
typedef uint32_t Stack_t;
#define RTOS_STACK_SIZE   256
#define RTOS_MAX_TASKS 16

#define port_PendSV_Handler PendSV_Handler
#define PortSVCHandler SVC_Handler
// 任务状态枚举
typedef enum
{
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
}task_state_t;

typedef void (*TaskEntry_t)(void *arg);

typedef struct task_tcb
{
    Stack_t *stack_top;// 当前栈顶指针
    Stack_t *stack_base;// 栈底指针
    Stack_t stack_size; // 栈大小
    uint8_t priority;
    task_state_t state;
    const char *name;
    uint32_t   wake_time; // 任务被阻塞时的唤醒时间（系统滴答数）
    uint32_t time_slice; // 任务的时间片长度（单位：系统滴答数）
    uint32_t time_slice_counter; // 任务已经使用的时间片计数器

    ListItem_t state_list_item; // 任务在就绪队列或延迟队列中的链表项

}TCB_T;

extern  TCB_T  * volatile  current_tcb;
extern  TCB_T  * volatile  next_tcb;

extern uint32_t task_count;

Stack_t *os_stack_init(Stack_t *stack_top, void (*task_func)(void *), void *arg);
int os_task_create(TCB_T *tcb,void (*entry)(void *),void *arg, uint32_t *stack_base,uint32_t stack_size, uint8_t priority);
void os_tick_init(uint32_t ticks_per_sec);
void os_start(void);
void yield(void);



#endif // !UOS_TASK_H_
