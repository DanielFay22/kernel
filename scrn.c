
#include <system.h>


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
        unsigned blank = 0x20 | (attrib << 8);
        unsigned temp;

        if (csr_y >= SCRN_H) {
                temp = csr_y - SCRN_H + 1;

                memcpy(textmemptr, textmemptr + SCRN_W * temp,
                    sizeof(unsigned short) * SCRN_W * (SCRN_H - temp));

                memsetw(textmemptr + SCRN_W * (SCRN_H - temp), blank,
                    SCRN_W * temp);
                csr_y = SCRN_H - 1;
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
        unsigned blank;
        int i;

        blank = 0x20 | (attrib << 8);

        for(i = 0; i < SCRN_H; i++)
                memsetw(textmemptr + i * SCRN_W, blank, SCRN_W);

        csr_x = 0;
        csr_y = 0;
        move_csr();
}

/* Puts a single character on the screen */
void
putch(char c)
{
        unsigned short *where;
        unsigned att = attrib << 8;

        /* Handle a backspace, by moving the cursor back one space */
        switch (c) {
        case 0x08:
                if(csr_x != 0) {
                        csr_x--;
                        where = textmemptr + (csr_y * SCRN_W + csr_x);
                        *where = 0x20 | att;
                }
                break;
        case 0x09:
                csr_x = (csr_x + 8) & ~(8 - 1);
                break;
        case '\r':
                csr_x = 0;
                break;
        case '\n':
                csr_x = 0;
                csr_y++;
                break;
        default:
                if(c >= ' ')
                {
                        where = textmemptr + (csr_y * SCRN_W + csr_x);
                        *where = c | att;	/* Character AND attributes: color */
                        csr_x++;
                }
                break;
        }


        /* If the cursor has reached the edge of the screen's width, we
        *  insert a new line */
        if(csr_x >= SCRN_W)
        {
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
        for (unsigned int i = 0; i < strlen(text); i++)
                putch(text[i]);
}

/* Sets the foreground and background color */
void
settextcolor(unsigned char forecolor, unsigned char backcolor)
{
        attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets text-mode VGA pointer, then clears the screen */
void
init_video(void)
{
        textmemptr = (unsigned short *)VGABUFFER;
        cls();
}
