


#include <system.h>



extern void swi0();
extern void swi1();
extern void swi2();
extern void swi3();
extern void swi4();
extern void swi5();
extern void swi6();
extern void swi7();
extern void swi8();
extern void swi9();
extern void swi10();
extern void swi11();
extern void swi12();
extern void swi13();
extern void swi14();
extern void swi15();


// Function handlers for all IRQ's.
void *swi_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void
swi_install_handler(int swi, void (* handler)(struct regs *r))
{
        swi_routines[swi] = handler;
}

void
swi_uninstall_handler(int swi)
{
        swi_routines[swi] = 0;
}


void
swi_install()
{
        // Setup IDT entries for each IRQ.
        idt_set_gate(0x80, (unsigned)swi0, 0x08, 0x8E);
        idt_set_gate(0x81, (unsigned)swi1, 0x08, 0x8E);
        idt_set_gate(0x82, (unsigned)swi2, 0x08, 0x8E);
        idt_set_gate(0x83, (unsigned)swi3, 0x08, 0x8E);
        idt_set_gate(0x84, (unsigned)swi4, 0x08, 0x8E);
        idt_set_gate(0x85, (unsigned)swi5, 0x08, 0x8E);
        idt_set_gate(0x86, (unsigned)swi6, 0x08, 0x8E);
        idt_set_gate(0x87, (unsigned)swi7, 0x08, 0x8E);
        idt_set_gate(0x88, (unsigned)swi8, 0x08, 0x8E);
        idt_set_gate(0x89, (unsigned)swi9, 0x08, 0x8E);
        idt_set_gate(0x8A, (unsigned)swi10, 0x08, 0x8E);
        idt_set_gate(0x8B, (unsigned)swi11, 0x08, 0x8E);
        idt_set_gate(0x8C, (unsigned)swi12, 0x08, 0x8E);
        idt_set_gate(0x8D, (unsigned)swi13, 0x08, 0x8E);
        idt_set_gate(0x8E, (unsigned)swi14, 0x08, 0x8E);
        idt_set_gate(0x8F, (unsigned)swi15, 0x08, 0x8E);
}



void
swi_handler(struct regs *r)
{
        // Blank function pointer.
        void (*handler)(struct regs *r);

        handler = swi_routines[r->int_no - 0x80];
        if (handler)
                handler(r);

}


