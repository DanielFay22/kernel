
#include <system.h>


#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


// Function handlers for all IRQ's.
void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void
irq_install_handler(int irq, void (* handler)(struct regs *r))
{
        irq_routines[irq] = handler;
}

void
irq_uninstall_handler(int irq)
{
        irq_routines[irq] = 0;
}

/*
 * Because IRQ's map to the same IDT entries as reserved interrupts,
 * remap them to unused interrupt entries.
 * TODO: Figure out wtf this is doing.
 */
void
irq_remap(void)
{
        outportb(PIC1_COMMAND, 0x11);
        outportb(PIC2_COMMAND, 0x11);
        outportb(PIC1_DATA, 0x20);
        outportb(PIC2_DATA, 0x28);
        outportb(PIC1_DATA, 0x04);
        outportb(PIC2_DATA, 0x02);
        outportb(PIC1_DATA, 0x01);
        outportb(PIC2_DATA, 0x01);
        outportb(PIC1_DATA, 0x00);
        outportb(PIC2_DATA, 0x00);
}




void
irq_install()
{
        // Remap routines to correct entries.
        irq_remap();

        // Setup IDT entries for each IRQ.
        idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
        idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
        idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
        idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
        idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
        idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
        idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
        idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
        idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
        idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
        idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
        idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
        idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
        idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
        idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
        idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}



void
irq_handler(struct regs *r)
{
        // Blank function pointer.
        void (*handler)(struct regs *r);

        handler = irq_routines[r->int_no - 32];
        if (handler)
                handler(r);

        // Check if need to notify slave controller.
        if (r->int_no >= 40)
                outportb(PIC2_COMMAND, PIC_EOI);

        // Notify master controller.
        outportb(PIC1_COMMAND, PIC_EOI);
}


