#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "stm32f4xx.h" // 根据你的芯片修改
#include <sys/types.h>
// 核心：让 printf 走串口
// int _write(int file, char *ptr, int len) {
//     for (int i = 0; i < len; i++) {
//         // 使用标准库发送字符，假设你初始化的是 USART1
//         USART_SendData(USART1, (uint8_t)ptr[i]);
//         // 等待发送完成
//         while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//     }
//     return len;
// }
// 补齐其他缺少的桩函数，消除链接器警告
int _read(int file, char *ptr, int len) { return 0; }
int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _close(int file) { return -1; }

// 如果你用了 malloc，还需要这
caddr_t _sbrk(int incr) {
    // 1. 声明链接脚本中定义的符号
    // 在链接脚本（.ld文件）中，通常会有 _end 或 end 符号标记堆的起始
    extern char _end; 
    
    static char *heap_end;
    char *prev_heap_end;

    // 2. 初始化 heap_end
    if (heap_end == 0) {
        heap_end = &_end;
    }

    prev_heap_end = heap_end;
    
    // 这里可以添加堆栈溢出检查逻辑（可选）
    // if (heap_end + incr > stack_ptr) { ... }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}