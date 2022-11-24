#ifndef __THREAD_H
#define __THREAD_H

#include <stdint.h>

typedef void thread_func(void *);//自定义通用函数模型，它将在很多线程函数中作为形参类型

enum task_status{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

/*中断栈intr_stack*/
//此结构用于中断发生时保护程序（线程或进程）的上下文环境
//进程或线程被外部中断或软中断打断时，会按照此结构压入上下文
//intr_exit中的出栈操作是此结构的逆操作
//此栈在线程自己的内核栈中位置固定，所在页的最顶端

struct intr_stack{
    uint32_t vec_no; //kernel.S 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; //esp会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /*以下由CPU从低特权级进入高特权级时加入*/
    uint32_t err_code; //err_code会被压入在eip之后
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void *esp;
    uint32_t ss;
};

/*线程栈thread_stack*/
//线程自己的栈，用于存储线程中待执行的函数
//此结构在线程自己的内核栈中位置不固定，用在switch_to时保存线程环境
//实际位置取决于实际运行情况
struct thread_stack{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /*线程第一次执行时，eip指向待调用的函数kernel_thread
    其它时候，eip是指向switch_to的返回地址*/
    void (*eip) (thread_func *func, void *func_arg);

    /*以下仅供第一次被调度上cpu时使用*/
    //参数unused_ret只为占位置充数为返回地址
    void (*unused_retaddr);
    thread_func *function; //由kernel_thread所调用的函数名
    void *func_arg; //由kernel_thread所调用的函数所需的参数
};

struct task_struct{
    uint32_t* self_kstack; //各内核线程都用自己的内核栈
    enum task_status status;
    uint8_t priority;
    char name[16];
    uint32_t stack_magic; //用这串数字做栈的边界标记，用于检测栈的溢出
}


#endif /*__THREAD_H*/