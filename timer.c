
#include <system.h>

#define CH1     0x40
#define CH2     0x41
#define CH3     0x42
#define CMD     0x43

#define BASE_CLOCK      1193180         // 1.19 MHz


const int timer_rate = 50;

static void
set_timer_rate(int hz)
{
        int val = BASE_CLOCK / hz;
        outportb(CMD, 0x36);
        outportb(CH1, val & 0xFF);
        outportb(CH1, val >> 8);
}


void
timer_handler(struct regs *r)
{
        static unsigned long tick_count = 0;
        tick_count++;

}


void
timer_install()
{
        // Set the timer rate to timer_rate Hz.
        set_timer_rate(timer_rate);

        irq_install_handler(0, timer_handler);
}