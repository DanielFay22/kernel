#include <system.h>

/* Defines an IDT entry */
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;     /* This will ALWAYS be set to 0! */
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr
{
    unsigned short limit;
    struct idt_entry *base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base,
    unsigned short sel, unsigned char flags)
{
        idt[num].sel = sel;
        idt[num].flags = flags;

        idt[num].base_lo = base & 0xFFFF;
        idt[num].base_hi = (base >> 16) & 0xFFFF;
}

/* Installs the IDT */
void idt_install()
{
        /* Setup the IDT pointer */
        idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
        idtp.base = &idt[0];

        /* Initialize IDT to zeros */
        memset(&idt, 0, sizeof(struct idt_entry) * 256);

        /* Points the processor's internal register to the new IDT */
        idt_load();
}