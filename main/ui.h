#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void check_stdscr_size(int height,int width);
void main_scrn(void);
void user_scrn(void);
void admin_scrn(void);
void about_scrn(void);

#endif
