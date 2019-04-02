#include <system.h>

void *
memcpy(void *dest, const void *src, size_t count)
{
        const char *sp = (const char *)src;
        char *dp = (char *)dest;
        for(; count != 0; count--) *dp++ = *sp++;
        return dest;
}

void *
memset(void *dest, char val, size_t count)
{
        char *temp = (char *)dest;
        for( ; count != 0; count--) *temp++ = val;
        return dest;
}

unsigned short *
memsetw(unsigned short *dest, unsigned short val, size_t count)
{
        unsigned short *temp = (unsigned short *)dest;
        for( ; count != 0; count--) *temp++ = val;
        return dest;
}

size_t
strlen(const char *str)
{
        size_t retval;
        for(retval = 0; *str != '\0'; str++) retval++;
        return retval;
}

// Assembly -- Reads from input port
unsigned char
inportb(unsigned short _port)
{
        unsigned char rv;
        __asm__ __volatile__("inb %1, %0" : "=a" (rv) : "dN" (_port));
        return rv;
}

// Assembly -- Writes to output port
void
outportb(unsigned short _port, unsigned char _data)
{
        __asm__ __volatile__("outb %1, %0" : : "dN" (_port), "a" (_data));
}

int
main()
{
        // Setup the GDT.
        gdt_install();

        // Setup IDT.
        idt_install();

        // Setup ISR's.
        isrs_install();

        // Setup IRQ handlers.
        irq_install();

        // Begin the system timer.
        timer_install();
        keyboard_init();

        // Allow interrupts to occur.
        __asm__ __volatile__("sti");

        // Setup the screen output and print to screen.
        init_video();
        puts("Hello World!\n");

        // Infinite loop.
        for (;;);


        // Added to prevent compiler errors
        return (1);
}