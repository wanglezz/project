#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"

#define IDT_DESC_CNT 0x21//目前总共支持的中断数
#define PIC_M_CTRL   0x20//主片控制端口
#define PIC_M_DATA   0x21//主片数据端口
#define PIC_S_CTRL   0xa0//从片控制端口
#define PIC_S_DATA   0xa1//从片数据端口

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

/*初始化可编程中断控制器8259A*/
static void pic_init()
{
    /*初始化主片*/
    outb (PIC_M_CTRL,0x11); //ICW1:边沿触发，级联8259A，需要ICW4
    outb (PIC_M_DATA,0x20); //ICW2:起始中断向量号为0x20，即IR[0-7]表示中断0x20~0x27
    outb (PIC_M_DATA,0x04); //ICW3:IR2接从片
    outb (PIC_M_DATA,0x01); //ICW4:8086模式，正常EOI

    /*初始化从片*/
    outb (PIC_S_CTRL,0x11); //ICW1:边沿触发，级联8259A，需要ICW4
    outb (PIC_S_DATA,0x28); //ICW2:起始中断向量号为0x28，即IR[8-15]表示中断0x28~0x2f
    outb (PIC_S_DATA,0x02); //ICW3:设置从片连接到主片的IR2引脚
    outb (PIC_S_DATA,0x01); //ICW4:8086模式，正常EOI

    /*打开主片上IR0，也就是目前只接受时钟产生的中断*/
    outb (PIC_M_DATA,0xfe);
    outb (PIC_S_DATA,0xff);

    put_str("   pic_init done\n");
}


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

/*完成有关中断的所有初始化工作*/
void idt_init()
{
    put_str("   idt_init start\n");
    idt_desc_init();        //初始化中断描述符表
    pic_init();             //初始化8259A

    /* 加载idt */
    uint64_t idt_operand = ((sizeof(idt)-1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("   idt_init done\n");

}