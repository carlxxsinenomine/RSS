#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"

//Generate user screen
void user_scr(void){
    int height,width;
    getmaxyx(stdscr,height,width);

    int window_width=width/2;

    WINDOW *win=newwin(height,window_width,0,width/4);
    if(!win){
        printf("Failed to load screen\n");
        exit(1);
    }

    wborder(win,'|','|','-','-','+','+','+','+');

    char exit[]="[X] Exit";
    mvwprintw(win,height-4,(window_width-strlen(exit))/2,"%s",exit);


    status_bar(win,"User");

    int ch=0;

    while(1){
        ch=wgetch(win);
        if(toupper(ch)==('X')){
            break;
        }
        check_winsize(win,height,window_width);
    }
    delwin(win);
}