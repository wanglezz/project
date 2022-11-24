#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "../device/timer.h"

void init_all(void)
{
    put_str("Init All\n");
    idt_init();
    timer_init();
}
