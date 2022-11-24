[bits 32]
%define ERROR_CODE nop      ;若在相关异常中CPU已经自动压入错误码,为保持栈中格式统一,这里不做操作

%define ZERO push 0         ;若在相关异常中CPU没有压入错误码,为了统一栈中格式,就手工压入一个0

extern idt_table

section .data
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text
intr%1entry:                ;中断处理程序入口

    %2                      ;接下来保存上下文环境
    push ds
    push es
    push fs
    push gs
    pushad                  ;PUSHAD压入32位寄存器，其入栈顺序是：eax,ecx,edx,ebx,esp,ebp,esi,sdi

    mov al,0x20
    out 0xa0,al
    out 0x20,al

    push %1                 ;不管idt_table中的处理程序是否需要参数都压入中断号便于调试

    call [idt_table + %1*4] ;调用idt_table中的C版本中断处理程序
    jmp intr_exit

section .data
    dd    intr%1entry       ;中断处理程序入口地址

%endmacro

section .text
global intr_exit
intr_exit:
    add esp, 4      ;跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4      ;跳过error_code
    iretd

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ERROR_CODE
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ZERO
VECTOR 0x19,ZERO
VECTOR 0x1a,ZERO
VECTOR 0x1b,ZERO
VECTOR 0x1c,ZERO
VECTOR 0x1d,ZERO
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO