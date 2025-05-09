#include <ncurses\ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"

/* @date_added: 04/10/2025
 * @return_type: void
 * @params: void
 * @description: Makes a new window for about screen
 */
void about_scr(void){
    int height,width;
    getmaxyx(stdscr,height,width);

    int window_width=width/2;

    WINDOW *win=newwin(height,window_width,0,width/4);
    if(!win){
        printf("Failed to load screen\n");
        exit(1);
    }

    wborder(win,'|','|','-','-','+','+','+','+');

    const char *about[]={
        "    _    ____   ___  _   _ _____ ",
        "   / \\  | __ ) / _ \\| | | |_   _|",
        "  / _ \\ |  _ \\| | | | | | | | |  ",
        " / ___ \\| |_) | |_| | |_| | | |  " ,
        "/_/   \\_\\____/ \\___/ \\___/  |_|  "
    };

    int about_row_size=sizeof(about)/sizeof(about[0]);
    for(int i=0;i<about_row_size;i++){
        int len=strlen(about[i]);
        int tab=(window_width-len)/2;
        mvwprintw(win,i+3,tab,"%s",about[i]);
    }

    const char *about_content[]={
        "The Room Scheduling System is an interactive scheduling program",
        "that gives access to everyone in Bicol University to check",
        "schedules of rooms. It utilizes ncurses—a header and a text-based",
        "user interface that primarily works on terminals across various",
        "operating systems.",
        "",
        "Additionally, with all the topics we learned from Computer",
        "Programming 1 to 2 classes, we implemented concepts such as loops,",
        "selection, sorting algorithms, structs, linked lists, pointers,",
        "and other relevant topics.",
        "",
        "This program has features such as a user interface, an admin page,",
        "and a user page. The user page allows viewing available room",
        "schedules across different buildings. The admin page allows users",
        "to add, edit, and delete room schedules.",
        "",
        "",
        "",
        "Authors:",
        "",
        "Margarata, Sean Eric",
        "Munoz, Carl Johannes",
        "Penetrante, Ethaniel James",
        "Santos, Gebhel Anselm",
        "Zuniga, Jay Mark"
    };

    int content_row_size=sizeof(about_content)/sizeof(about_content[0]);
    for(int i=0;i<content_row_size;i++){
        int len=strlen(about_content[i]);
        mvwprintw(win,((height-content_row_size)/2)+i,(window_width-len)/2,"%s",about_content[i]);
    }

    char exit[]="[X] Exit";
    mvwprintw(win,height-4,(window_width-strlen(exit))/2,"%s",exit);

    status_bar(win,"Menu/About");

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
