
#include <stddef.h>


#ifndef __SYSTEM_H
#define __SYSTEM_H

/* MAIN.C */
void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, char val, size_t count);
unsigned short *memsetw(unsigned short *dest,
    unsigned short val, size_t count);
size_t strlen(const char *str);
unsigned char inportb (unsigned short _port);
void outportb (unsigned short _port, unsigned char _data);



/* VGA headers and macros */
void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();


// Functions for GDT.
//extern void gdt_set_gate(int num,
//    unsigned long base, unsigned long limit,
//    unsigned char access, unsigned char gran);
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



// Keyboard headers
void keyboard_init();

#endif
