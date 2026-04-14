/* rtos_asm.s —— PendSV 任务切换，GCC ARM 汇编 */
    .syntax unified
    .thumb

    .extern current_tcb
    .extern next_tcb
    .global PendSV_Handler
    .type PendSV_Handler, %function

/* ──────────────────────────────────────────────────────
 * PendSV_Handler
 * 进入时：CPU 已经把 xPSR/PC/LR/R12/R3/R2/R1/R0 压到
 *         当前任务的 PSP 栈上了（硬件自动做的）
 * 我们要做：
 *   1. 手动把 R11-R4 也压栈
 *   2. 把当前 PSP 保存到 current_tcb->sp
 *   3. 切换 current_tcb = next_tcb
 *   4. 从 next_tcb->sp 恢复 PSP
 *   5. 弹出 R11-R4
 *   6. 异常返回，硬件自动弹出 R0-R3/R12/LR/PC/xPSR
 * ────────────────────────────────────────────────────── */
    .thumb_func
PendSV_Handler:
    /* 关中断，保证切换原子性 */
    CPSID   I

    /* 读当前任务 PSP */
    MRS     R0, PSP
    ISB

    /* 手动压栈 R4-R11（低地址方向） */
    STMDB   R0!, {R4-R11}

    /* 保存新的栈顶到 current_tcb->sp
     * current_tcb 是指针，*current_tcb 的第一个字段就是 sp */
    LDR     R1, =current_tcb
    LDR     R1, [R1]          /* R1 = current_tcb（TCB地址）*/
    STR     R0, [R1]          /* current_tcb->sp = R0 */

    /* 切换：current_tcb = next_tcb */
    LDR     R2, =next_tcb
    LDR     R2, [R2]
    LDR     R3, =current_tcb
    STR     R2, [R3]

    /* 从新任务的 TCB 取出栈顶 */
    LDR     R0, [R2]          /* R0 = next_tcb->sp */

    /* 弹出 R4-R11 */
    LDMIA   R0!, {R4-R11}

    /* 更新 PSP 到弹出后的新位置 */
    MSR     PSP, R0
    ISB

    /* 开中断 */
    CPSIE   I

    /* 异常返回：LR = 0xFFFFFFFD
     * 硬件自动从 PSP 弹出剩余8个寄存器，跳到新任务 PC */
    BX      LR

    .end
