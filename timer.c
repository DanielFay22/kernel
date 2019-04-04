
#include <system.h>

#define CH1     0x40
#define CH2     0x41
#define CH3     0x42
#define CMD     0x43

#define BASE_CLOCK      1193180         // 1.19 MHz


const int timer_rate = 100;
unsigned long tick_count = 0;


struct count {
    volatile int count;
    struct count *next;
};

struct count *head = NULL;


static struct count *add_wait_time(int ms);


/*
 * Initialize timer to configured rate.
 */
static void
set_timer_rate(int hz)
{
        int val = BASE_CLOCK / hz;
        outportb(CMD, 0x36);
        outportb(CH1, val & 0xFF);
        outportb(CH1, val >> 8);
}


/*
 * Handler to maintain a running count of the number of timer
 * ticks that have occurred.
 */
void
timer_handler(struct regs *r)
{
//        if (tick_count++ % 100 == 0)
//                puts("One Second\n");
        struct count *c = head;
        while (c != NULL) {
                c->count--;
                c = c->next;
        }
}

static struct count *
add_wait_time(int ms)
{
        struct count *c = malloc(sizeof(struct count));
        if (c == NULL) {
                puts("Out of Memory\n");
                return NULL;
        }

        c->count = ms;
        c->next = head;

        // Block hardware interrupts while writing.
//        BLOCK_INT;
        head = c;
//        ENABLE_INT;

        return c;
}

static void
remove_wait_time(struct count *c)
{
        struct count *cp = head;
        while (cp != NULL && cp->next != c)
                cp = cp->next;

        if (cp == NULL)
                return;

        cp->next = c->next;
        free(c);
}

void
sleep(int ms)
{
        struct count *c = add_wait_time(ms / 10);


        while (c->count > 0);

        remove_wait_time(c);
}






/*
 * Initialize the timer and setup the IRQ handler.
 * The timer triggers on IRQ 0.
 */
void
timer_install()
{
        // Set the timer rate to timer_rate Hz.
        set_timer_rate(timer_rate);

//        int f = BASE_CLOCK / 500;
//        outportb(CMD, 0xB6);
//        outportb(CH2, f & 0xFF);
//        outportb(CH2, f >> 8);

        irq_install_handler(0, timer_handler);
}