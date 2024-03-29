#include <system.h>


struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr
{
    unsigned short limit;
    struct gdt_entry *base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush();


/* Setup a descriptor in the Global Descriptor Table */
void
gdt_set_gate(int num, unsigned long base, unsigned long limit,
    unsigned char access, unsigned char gran)
{
        /* Setup the descriptor base address */
        gdt[num].base_low = (base & 0xFFFF);
        gdt[num].base_middle = (base >> 16) & 0xFF;
        gdt[num].base_high = (base >> 24) & 0xFF;

        /* Setup the descriptor limits */
        gdt[num].limit_low = (limit & 0xFFFF);
        gdt[num].granularity = ((limit >> 16) & 0x0F);

        /* Finally, set up the granularity and access flags */
        gdt[num].granularity |= (gran & 0xF0);
        gdt[num].access = access;
}

/*
 *  Should be called by main. Sets up GDT pointer,
 *  sets up the first 3 entries in GDT, and then
 *  finally calls gdt_flush() in start.asm in order
 *  to tell the processor where the new GDT is and update the
 *  new segment registers.
 */
void
gdt_install()
{
        /* Setup the GDT pointer and limit */
        gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
        gp.base = &gdt[0];

        /* NULL descriptor */
        gdt_set_gate(0, 0, 0, 0, 0);

        /* Code Segment */
        gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

        /* Data Segment */
        gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

        /* Flush out the old GDT and install the new changes */
        gdt_flush();
}