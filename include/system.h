
#include <stddef.h>
#include <malloc.h>
#include <scrn.h>


#ifndef __SYSTEM_H
#define __SYSTEM_H

#define BLOCK_INT       __asm__ __volatile__ ("cli")
#define ENABLE_INT      __asm__ __volatile__ ("sti")


#define GetInInterrupt(arg) __asm__("int %0\n" : : "N"((arg)) : "cc", "memory")

/* MAIN.C */
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, char val, size_t count);
unsigned short *memsetw(unsigned short *dest,
    unsigned short val, size_t count);
unsigned char inportb(unsigned short _port);
void outportb(unsigned short _port, unsigned char _data);



// Functions for GDT.
void gdt_install();



// IDT and ISR headers
void idt_set_gate(unsigned char num, unsigned long base,
    unsigned short sel, unsigned char flags);
void idt_install();


// struct representing stack after ISR execution.
struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

void isrs_install();


// Headers for IRQ handlers.
void irq_install();
void irq_install_handler(int irq, void (* handler)(struct regs *r));
void irq_uninstall_handler(int irq);



// System clock headers
void timer_install();
extern void sleep(int ms);



// Keyboard headers
void keyboard_init();
char *line_buffer;


void push_screen(struct regs *r);



void swi_install_handler(int swi, void (* handler)(struct regs *r));
void swi_uninstall_handler(int swi);
void swi_install();



// Memory headers
//void vmem_init();
//uint32_t *pd;

#endif
