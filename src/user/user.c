#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"

/* @date_added: 04/15/2025
 * @return_type: void
 * @params: void
 * @description: Makes a new window for user screen
 */
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

    char exit_x[]="[X] Exit";
    mvwprintw(win,height-4,(window_width-strlen(exit_x))/2,"%s",exit_x);

    status_bar(win,"User");

    const char *bldng[]={
	    " ___  _  _ _ _    ___  _ _  _ ____ ____  ", 
	    "|__] |  | | |    |  \\ | |\\ | | __ [__ ",  
	    "|__] |__| | |___ |__/ | | \\| |__] ___] ",
    };

    int bldng_row_size=sizeof(bldng)/sizeof(bldng[0]);
    for(int i=0;i<bldng_row_size;i++){
        int len=strlen(bldng[i]);
        int tab=(window_width-len)/2;
        mvwprintw(win,i+3,tab,"%s",bldng[i]);
    }

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
