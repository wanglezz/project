#include "interrupt.h"
#include "stdint.h"
#include "global.h"

#define IDT_DESC_CNT 0x21//目前总共支持的中断数

/*中断门描述符结构体*/
struct gate_desc{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount;//此项为双字计数字段，是门描述符的第四字节，为固定值不用考虑
    uint8_t attribute;
    uint16_t func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc,uint8_t attr,intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];//idt是中断描述符表，本质上是个中断门描述符数组

extern intr_handler intr_entry_table[IDT_DESC_CNT];//定义在kernel.S中的中断处理程序入口数组


/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr,intr_handler function)
{
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

/* 初始化中断描述符表 */
static void idt_desc_init(void)
{
    int i;
    for(i=0;i<IDT_DESC_CNT;i++)
    {
        make_idt_desc(&idt[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
    }
    put_str("   idt_desc_init done\n");
}

void idt_init()
{
    put_str("   idt_init start\n");
    idt_desc_init();
    pic_init();

    /* 加载idt */
    uint64_t idt_operand = ((sizeof(idt)-1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("   idt_init done\n");

}