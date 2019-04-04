/*
 * The virtual memory system code goes in here.
 */

#include <system.h>



// Function in start.asm that enables MMU.
extern void paging_init();



// Pointer to start of page directory.
uint32_t page_directory[1024]__attribute__((aligned(4096)));
uint32_t page_table[1024]__attribute__((aligned(4096)));



/*
 * Page Directory Entry:
 * ---------------------
 * Bit 0        -       Set to indicate entry present.
 * Bit 1        -       Set to enable read/write, clear to make read-only.
 * Bit 2        -       Set to allow access from all users.
 * Bit 3        -       Set to enable write-through caching.
 * Bit 4        -       Set to disable page caching.
 * Bit 5        -       Indicates if page has been accessed.
 * Bit 6        -       0.
 * Bit 7        -       Set to enable 4 MiB pages.
 * Bit 8        -       Ignored.
 * Bits 9-11    -       Available for OS use.
 * Bits 12-31   -       Page table address (4 KiB aligned).
 *
 *
 * Page Table Entry:
 * ---------------------
 * Bits 0-5     -       Same as above.
 * Bit 6        -       Set to indicate page was written to.
 * Bit 7        -       0.
 * Bit 8        -       If set, TLB when not update address in cache if CR3
 *                      is reset. If page global enable bit is not set in CR4,
 *                      has no effect.
 * Bit 9-31     -       Same as above.
 *
 */




/*
 * Sets up tables for virtual memory system.
 * Note this must be called before switching to virtual mode.
 */
void
vmem_init()
{
        int i;

        // Initialize the Page Directory.
        for (i = 0; i < 1024; i++)
                page_directory[i] = 0x2;

        for (i = 0; i < 1024; i ++)
                page_table[i] = (i * 0x1000) | 3;

        page_directory[0] = ((unsigned int)page_table) | 3;

        pd = (uint32_t *)page_directory;

        // Point last entry to page directory.
        page_directory[1023] = ((unsigned int)page_directory[0]) | 3;

        paging_init();
}