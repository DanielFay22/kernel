
#include <system.h>
#include <string.h>


enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};


#define VGABUFFER       0xB8000
#define SCRN_H          25
#define SCRN_W          80

unsigned short *textmemptr;
int attrib = VGA_COLOR_WHITE;
int csr_x = 0, csr_y = 0;


void
scroll(void)
{
//        unsigned int temp;

        if (csr_y >= SCRN_H) {
//                temp = csr_y - SCRN_H + 1;
                memcpy(textmemptr, textmemptr + (SCRN_H - 1) * SCRN_W,
                    sizeof(unsigned short) * SCRN_W);
                memset(textmemptr + SCRN_W, 0x00,
                    (SCRN_H - 1) * SCRN_W * sizeof(unsigned short));
                csr_y = 1;
                csr_x = 0;

//                memcpy(textmemptr, textmemptr + SCRN_W * temp,
//                    sizeof(unsigned short) * SCRN_W * (SCRN_H - temp));
//
//                // Zero out the "newly added" lines.
//                memset(textmemptr + SCRN_W * (SCRN_H - temp),
//                    0x00, SCRN_W * temp * sizeof(unsigned short));
//
//                csr_y = SCRN_H - 1;
        }
}

/* Updates the hardware cursor */
void
move_csr(void)
{
        unsigned temp = csr_y * SCRN_W + csr_x;

        outportb(0x3D4, 14);
        outportb(0x3D5, temp >> 8);
        outportb(0x3D4, 15);
        outportb(0x3D5, temp);
}

/* Clears the screen */
void cls()
{
        memsetw(textmemptr, 0x20 | (attrib << 8), SCRN_W * SCRN_H);

        csr_x = 0;
        csr_y = 0;
        move_csr();
}

/* Puts a single character on the screen */
void
putch(char c)
{
        unsigned short att = attrib << 8;

        switch (c) {
        case 0x08:      // Backspace
                if(csr_x != 0) {
                        csr_x--;
                        *(textmemptr + (csr_y * SCRN_W + csr_x)) = 0x00;
                } else if (csr_y != 0) {
                        csr_y--;
                        csr_x = SCRN_W - 1;
                        *(textmemptr + (csr_y * SCRN_W + csr_x)) = 0x00;
                }
                break;
        case 0x09:      // TAB
                csr_x = (csr_x + 8) & ~(8 - 1);
                break;
        case '\r':
                csr_x = 0;
                break;
        case '\n':
                // Zero out row
                *(textmemptr + csr_x + SCRN_W * csr_y) = '\n';
                memset(textmemptr + csr_x + csr_y * SCRN_W + 1, 0x00,
                       sizeof(unsigned short) * (SCRN_W - csr_x));
                csr_x = 0;
                csr_y++;
                break;
        default:
                if (c >= ' ') {
                        *(textmemptr + (csr_y * SCRN_W + csr_x)) = c | att;
                        csr_x++;
                }
                break;
        }


        /* If the cursor has reached the edge of the screen's width, we
        *  insert a new line */
        if (csr_x == SCRN_W - 1) {
                *(textmemptr + csr_x + SCRN_W * csr_y) = '\n';
                csr_x = 0;
                csr_y++;
        } else if (csr_x >= SCRN_W) {
                csr_x = 0;
                csr_y++;
        }

        /* Scroll the screen if needed, and finally move the cursor */
        scroll();
        move_csr();
}

/* Puts a NULL terminated string on the screen */
void
puts(char *text)
{
        while (*text != '\0')
                putch(*text++);
}

void
putnum(unsigned int num)
{
        static char *digits = "0123456789";

        while (num > 0) {
                putch(digits[num % 10]);
                num /= 10;
        }
}

/* Sets the foreground and background color */
void
settextcolor(unsigned char forecolor, unsigned char backcolor)
{
        attrib = (backcolor << 4) | (forecolor & 0x0F);
}

void
getline(unsigned int y, char *buf)
{
        unsigned int c;
        for (c = 0; c < SCRN_W - 1; c++) {
                *(buf + c) = (char) (*(textmemptr + c + y * SCRN_W) & 0xFF);
                if (*(buf + c) == '\n') {
                        *(buf + c + 1) = '\0';
                        return;
                }
        }

        *(buf + SCRN_W - 1) = '\0';
        return;
}

void
getcurline(char *buf)
{
        getline((unsigned int)csr_y, buf);
}

void
getprevline(char *buf)
{
        if (csr_y > 0)
                getline((unsigned int)(csr_y - 1), buf);
        else
                getline(0, buf);
}


void
push_screen(struct regs *r)
{
        memcpy((unsigned short *)VGABUFFER, textmemptr,
               sizeof(unsigned short) * SCRN_W * SCRN_H);
}


/* Sets text-mode VGA pointer, then clears the screen */
void
init_video(void)
{
        textmemptr = (unsigned short *)VGABUFFER;
//            (unsigned short *)malloc(
//            sizeof(unsigned short) * SCRN_W * SCRN_H);

//        install_timer_interrupt(5, push_screen);

        cls();
}
