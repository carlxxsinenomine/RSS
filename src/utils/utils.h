#ifndef UTILS_H
#define UTILS_H

#include <ncurses.h>

void check_winsize(WINDOW *win,int height,int width);
void main_scr(void);
void user_scr(void);
void admin_scr(void);
void about_scr(void);

#endif