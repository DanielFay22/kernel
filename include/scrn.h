

#ifndef __SCRN_H
#define __SCRN_H

void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();

void getprevline(char *buf);
void getline(unsigned int y, char *buf);
void getcurline(char *buf);

void putnum(unsigned int num);

#endif //KERNEL_SCRN_H
