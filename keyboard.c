
#include <system.h>

/*
 * Codes:
 *
 * a            -       30, 158
 * b            -       48, 176
 * c            -       46, 174
 * d            -       32, 160
 * e            -       18, 146
 * f            -       33, 161
 * g            -       34, 162
 * h            -       35, 163
 * i            -       23, 151
 * j            -       36, 164
 * k            -       37, 165
 * l            -       38, 166
 * m            -       50, 178
 * n            -       49, 177
 * o            -       24, 152
 * p            -       25, 153
 * q            -       16, 144
 * r            -       19, 147
 * s            -       31, 159
 * t            -       20, 148
 * u            -       22, 150
 * v            -       47, 175
 * w            -       17, 145
 * x            -       45, 173
 * y            -       21, 149
 * z            -       44, 172
 *
 *
 * 1            -       2, 130
 * 2            -       3, 131
 * 3            -       4, 132
 * 4            -       5, 133
 * 5            -       6, 134
 * 6            -       7, 135
 * 7            -       8, 136
 * 8            -       9, 137
 * 9            -       10, 138
 * 0            -       11, 139
 * -            -       12, 140
 * =            -       13, 141
 * back         -       14, 142
 *
 * ;            -       39, 167
 * '            -       40, 168
 * space        -       57, 185
 * tab          -       15, 143
 * esc          -       1, 129
 *
 * shift        -       42, ..., 170
 * ctrl         -       29, ..., 157
 * alt          -       56, ..., 184
 *
 * `            -       41, 169
 * \            -       43, 171
 * ,            -       51, 179
 * .            -       52, 180
 * /            -       53, 181
 * F1-F10       -       58 + n, 186 + n
 * F11          -       87, 215
 * F12          -       88, 216
 *
 */

// Keyboard port
#define KEYBOARD_PORT   0x60

// A few response bytes
#define         RESET   0xFF
#define         ACK     0xFA
#define         RESEND  0xFE
#define         ECHO    0xEE


char key_states[256];
char *lookup = "__1234567890-=\b\tqwertyuiop[]\n_asdfghjkl;'`_\\zxcvbnm,./"
               "___ ";
char *shift_lookup = "__!@#$%^&*()_+\b\tQWERTYUIOP{}\n_ASDFGHJKL:\"~_|ZXCVB"
                     "NM<>?___ ";

// Future: used to store commands to be sent on exit from IRQ.
char command_queue[32];


void
keypress_handler(struct regs *r)
{
        // Read incoming character.
        unsigned char c = inportb(KEYBOARD_PORT);

        if (key_states[0xE0] > 0) {
                // TODO: Arrow keys.
                key_states[0xE0] = 0;
                return;
        }

        if (c > 128) {
                if (c == 0xE0)
                        key_states[c] = 1;
                else {
                        c -= 128;
                        key_states[c] = 0;
                }
                return;
        }

        char *l = lookup;
        if (key_states[42])
                l = shift_lookup;


        key_states[c] = 1;

        switch (c) {
        case 42:        // Shift
        case 29:        // Ctrl
        case 56:        // Alt
                break;

        case 1:         // esc
                // Esc command
                break;

        // Function keys.
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 68:
        case 87:
        case 88:
                // Function handler
                break;

        default:
                if (c <= 57)
                        putch(l[c]);

        }
}


void
keyboard_init()
{
        for (int i = 0; i < 256; i ++)
                key_states[i] = 0;

        for (int i = 0; i < 32; i ++)
                command_queue[i] = 0;

        irq_install_handler(1, keypress_handler);
}