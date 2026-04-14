#include "uos_task.h"
#include "stm32f4xx.h"




TCB_T * volatile current_tcb = 0;
TCB_T * volatile next_tcb = 0;
static TCB_T idle_task_tcb;
static uint32_t idle_task_stack[128];

uint32_t task_count = 0;
static List_t g_ready_list[OS_MAX_PRI];
static List_t g_delay_list;
static bool init = false;
volatile uint32_t g_os_tick = 0;
static void os_sched_init(void)
{
    list_init(&g_delay_list);
    for(int i = 0; i < OS_MAX_PRI; ++i)
    {
        list_init(&g_ready_list[i]);
    }
}

static void os_idle_task(void *arg)
{
    (void)arg;
    while (1)
    {
        __WFI();   // 或者 nop,进入低功耗等待中断状态，直到有任务需要调度
    }
}

static void os_ready_insert(TCB_T *tcb)
{
    if(tcb == NULL)
    {
        return;
    }

    if(tcb->state != TASK_READY)
    {
        return;
    }
    list_insert_tail(&g_ready_list[tcb->priority], &tcb->state_list_item);
}
static void os_ready_remove(TCB_T *tcb)
{
    if(tcb == NULL)
    {
        return;
    }
    list_remove(&tcb->state_list_item);
}
static void os_context_switch(TCB_T *tcb)
{
    if(tcb == NULL || tcb == current_tcb)
    {
        return;
    }
    next_tcb = tcb;

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // 触发PendSV中断，进行上下文切换
    __DSB(); // 数据同步屏障，确保指令执行顺序
    __ISB(); // 指令同步屏障，确保中断处理完成后再执行后续指令

}
static void AddToDelayList(TCB_T *tcb,uint32_t ticks){
    if(tcb == NULL || ticks == 0)
    {
        return;
    }
    tcb->wake_time = g_os_tick + ticks;
    tcb->state_list_item.item_value = tcb->wake_time; // 用于有序插入
    list_remove(&tcb->state_list_item); // 从就绪队列移除
    list_insert_ordered(&g_delay_list, &tcb->state_list_item); // 插入延迟队列
}
/* PendSV Handler: 任务切换的核心 */
__attribute__((naked)) void port_PendSV_Handler(void)
{
    __asm volatile
    (
        "mrs R0, psp                \n"
        "STMDB R0!, {R4-R11}         \n"

        "LDR R1, pxCurrentTCBConst        \n"
        "LDR R1, [R1]               \n"
        "STR R0, [R1]               \n"

        "LDR     R2, pxCurrentTCBConst1       \n" /* 获取 next_tcb 变量的地址 */
        "LDR     R2, [R2]            \n" /* 读出里面的值 (Task B 的 TCB 首地址) */
        "LDR     R3, pxCurrentTCBConst    \n"  /* 获取 current_tcb 变量的地址 */
        "STR     R2, [R3]            \n"

        "LDR     R0, [R2] \n"
        "LDMIA   R0!, {R4-R11}\n"
        "MSR     PSP, R0\n"
        "BX      LR\n"
        "pxCurrentTCBConst: .word current_tcb	\n"
        "pxCurrentTCBConst1: .word next_tcb	\n"

    );
}
__attribute__((naked)) static void PortStartFirstTask( void )
{

	__asm volatile(
					" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
					" ldr r0, [r0] 			\n"
					" ldr r0, [r0] 			\n"
					" msr msp, r0			\n" /* Set the msp back to the start of the stack. */
					" cpsie i				\n" /* Globally enable interrupts. */
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n"
					" svc 0					\n" /* System call to start first task. */
					" nop					\n"
				);
}
__attribute__( ( naked ) ) void PortSVCHandler( void )
{
    __asm volatile (
        /* 1. 拿到第一个任务的 TCB 和 栈顶 */
        "   ldr r3, pxCurrentTCBConst2      \n"
        "   ldr r1, [r3]                    \n"
        "   ldr r0, [r1]                    \n" /* r0 = pxCurrentTCB->pxTopOfStack */

        /* 2. 手动弹出那 8 个不需要硬件操心的寄存器 R4-R11 */
        "   ldmia r0!, {r4-r11}             \n"

        /* 3. 把真正的栈顶位置交给 PSP */
        "   msr psp, r0                     \n"
        "   isb                             \n"

        /* 4. 清除中断掩码（为了严谨） */
        "   mov r0, #0                      \n"
        "   msr basepri, r0                 \n"

        /* 5. [终极魔法] 伪造异常返回状态码 */
        "   orr r14, #0xd                   \n" /* 把 LR 寄存器的最低 4 位置为 1101，即 0xFFFFFFFD */

        /* 6. 异常返回！硬件自动接管！ */
        "   bx r14                          \n"
        "                                   \n"
        "   .align 4                        \n"
        "pxCurrentTCBConst2: .word current_tcb	\n"
    );
}

Stack_t *os_stack_init(Stack_t *stack_top,
                       void (*entry)(void *), void *arg)
{
    uint32_t *sp = stack_top;

    *(--sp) = 0x01000000;          // xPSR, T bit必须为1
    *(--sp) = (uint32_t)entry;     // PC
    *(--sp) = 0xFFFFFFFD;          // LR, 异常返回到Thread模式，用PSP
    *(--sp) = 0x12121212;          // R12
    *(--sp) = 0x03030303;          // R3
    *(--sp) = 0x02020202;          // R2
    *(--sp) = 0x01010101;          // R1
    *(--sp) = (uint32_t)arg;       // R0

    *(--sp) = 0x11111111;          // R11
    *(--sp) = 0x10101010;          // R10
    *(--sp) = 0x09090909;          // R9
    *(--sp) = 0x08080808;          // R8
    *(--sp) = 0x07070707;          // R7
    *(--sp) = 0x06060606;          // R6
    *(--sp) = 0x05050505;          // R5
    *(--sp) = 0x04040404;          // R4

    return sp;
}
int os_task_create(TCB_T *tcb,void (*entry)(void *),void *arg, uint32_t *stack_base,uint32_t stack_size, uint8_t priority)
{
    if(!tcb || !stack_base || stack_size == 0)
    {
        return -1; // 参数错误
    }
    list_item_init(&tcb->state_list_item);

    if(init == false)
    {
        os_sched_init();
        init = true;

    }

    if (priority >= OS_MAX_PRI)
    {
        return -1; // 优先级越界
    }
    if(task_count >= RTOS_MAX_TASKS)
    {
        return -1; // 任务数超过最大限制
    }

    uint32_t stack_top = (uint32_t)(stack_base + stack_size);

    tcb->stack_top = os_stack_init((Stack_t *)stack_top, entry, arg);
    tcb->stack_base = (Stack_t *)stack_base;
    tcb->stack_size = stack_size;
    tcb->priority = priority;
    tcb->state = TASK_READY;
    tcb->wake_time = 0U;
    tcb->time_slice_counter= 10U;
    tcb->state_list_item.owner = tcb;

    os_ready_insert(tcb);
    task_count++;


    return 0; // 创建成功
}
//
static TCB_T *os_sched_select_next(void)
{
    int prio;
    ListItem_t *item;

    for (prio = OS_MAX_PRI - 1; prio >= 0; prio--)
    {
        if (!list_is_empty(&g_ready_list[prio]))
        {
            item = list_get_head(&g_ready_list[prio]);
            if (item->owner != NULL)
            {
                return (TCB_T *)item->owner;
            }
        }
    }

    return NULL;
}

static void os_wake_expired_tasks(void)
{
    ListItem_t *item;
    while(!list_is_empty(&g_delay_list))
    {
        item = list_get_head(&g_delay_list);
        if(item ==NULL) break;

        TCB_T *tcb = (TCB_T *)item->owner;
        if(tcb->wake_time > g_os_tick)
        {
            break;
        }
        list_remove(&tcb->state_list_item);
        tcb->state = TASK_READY;
        os_ready_insert(tcb);
    }
}
static void os_schedule(bool rotate_current)
{
    TCB_T *cand;
    TCB_T *prev = current_tcb;

    if (rotate_current && prev != NULL && prev->state == TASK_RUNNING)
    {
        prev->state = TASK_READY;
        os_ready_remove(prev);
        os_ready_insert(prev);
    }

    cand = os_sched_select_next();
    if (cand == NULL)
    {
        return;
    }

    /* 非轮转抢占路径：旧任务从 RUNNING -> READY（不在这里做队列旋转） */
    if (!rotate_current && prev != NULL && prev != cand && prev->state == TASK_RUNNING)
    {
        prev->state = TASK_READY;
    }

    cand->state = TASK_RUNNING;
    os_context_switch(cand);
}


void yield(void)
{
    os_schedule(true);

}

void os_start(void)
{

    if(!init)
    {
        os_sched_init();
        init = true;
    }

    os_task_create(&idle_task_tcb, os_idle_task, NULL, idle_task_stack, 128, 0); // 创建空闲任务，优先级最低
    if (task_count == 0)
    {
        while (1) {}
    }

    current_tcb = os_sched_select_next();
    next_tcb = current_tcb;
    current_tcb->state = TASK_RUNNING;
    os_tick_init(1000); // 初始化系统滴答定时器，1ms中断一次


    PortStartFirstTask();

    while (1) {}
}
void os_tick_init(uint32_t ticks_per_sec)
{
    NVIC_SetPriority(SysTick_IRQn, 0x0E); // 设置 SysTick
    NVIC_SetPriority(PendSV_IRQn, 0x0F); // 设置 PendSV 中断优先级为最低
    SysTick_Config(SystemCoreClock / ticks_per_sec);

}


void SysTick_Handler(void)
{
    TCB_T *cur_tcb = current_tcb;
    TCB_T *next;

    g_os_tick++;
    os_wake_expired_tasks();

    if(cur_tcb == NULL)
    {
        os_schedule(false);
        return;
    }

    if(cur_tcb->time_slice > 0)
    {
        cur_tcb->time_slice--;
    }

    next = os_sched_select_next();
    if(next == NULL)
    {
        return;
    }

    // 如果当前任务没有时间片或者有更高优先级的任务就切换
    if(next->priority > cur_tcb->priority)
    {
        os_schedule(false);
        return;
    }

    // 时间片用完或者有更高优先级的任务就切换
    if(next->priority > cur_tcb->priority ||(cur_tcb->priority == next->priority && cur_tcb->time_slice == 0) )
    {
        if(cur_tcb->time_slice == 0)
        {
            cur_tcb->time_slice = cur_tcb->time_slice_counter; // 重置时间片
            os_schedule(true); // 同优先级轮转
        }
    }

}
void os_delay(uint32_t ticks)
{
    TCB_T *next;
    if(current_tcb == NULL || ticks == 0)
    {
        return;
    }

    current_tcb->state = TASK_BLOCKED;
    AddToDelayList(current_tcb, ticks);

    next = os_sched_select_next();
    if(next == NULL)
    {
        return;
    }
        next->state = TASK_RUNNING;
        os_context_switch(next);
}
