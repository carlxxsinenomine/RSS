#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//Function initialization
void main_scrn(void);
void user_scrn(void);
void admin_scrn(void);
void about_scrn(void);

//Generate user screen
void user_scrn(void){

}

//Generate admin screen
void admin_scrn(void){

}

//Generate about screen
void about_scrn(void){
	int height, width;
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

	for(int i=0;i<5;i++){
		int len=strlen(about[i]);
		int tab=(window_width-len)/2;
		mvwprintw(win,i+3,tab,"%s",about[i]);
	}

	mvwprintw(win,height-3,(window_width-9)/2,"[X] Exit");

	const char *about_content[]={
		"Authors:",
		"",
		"Margarata, Sean Eric A.",
		"Muñoz, Carl Johannes",
		"Penetrante, Ethaniel James",
		"Santos, Gebhel Anselm",
		"Zuñiga, Jay Mark"
	};

	for(int i=0;i<7;i++){
		int len=strlen(about_content[i]);
		mvwprintw(win,((height-7)/2)+i,(window_width-len)/2,"%s",about_content[i]);
	}

	int ch=0;
	while(1){
		ch=wgetch(win);
		if(toupper(ch)==('X')){
			break;
		}
	}
	
	wrefresh(win);

	delwin(win);
}

//Generate menu screen
void main_scrn(void){
	noecho();
	curs_set(0);

	int height, width;
	getmaxyx(stdscr,height,width);

	int window_width=width/2;


	WINDOW *win=newwin(height,window_width,0,width/4);
	if(!win){
		printf("Failed to load screen\n");
		exit(1);
	}

	wborder(win,'|','|','-','-','+','+','+','+');
	
	const char *title[]={
		"8888888b.  .d8888b.  .d8888b. ",
		"888   Y88bd88P  Y88bd88P  Y88b",
		"888    888Y88b.     Y88b.     ",
		"888   d88P \"Y888b.   \"Y888b.  ",
		"8888888P\"     \"Y88b.    \"Y88b.",
		"888 T88b        \"888      \"888",
		"888  T88b Y88b  d88PY88b  d88P",
		"888   T88b \"Y8888P\"  \"Y8888P\" ",
		"",
		"An interactive room scheduling system built using",
		"Ncurses in seek of room management and availability directly from the terminal.",
	};

	for(int i=0;i<11;i++){
		if(i==0){
			wattrset(win,A_BOLD);
		}
		else if(i==9){
			wattrset(win,A_ITALIC);
		}

		int len=strlen(title[i]);
		int tab=(window_width-len)/2;
		mvwprintw(win,i+7,tab,"%s",title[i]);
	}
	wattrset(win,A_NORMAL);

	const char *menu[]={
		"[1] User",
		"[2] Admin",
		"[3] About",
		"[4] Exit"
	};

	for(int i=0;i<4;i++){
		int len=strlen(menu[i]);
		mvwprintw(win,(height/2)+i*2,(window_width-len)/2,"%s",menu[i]);
	}

	wrefresh(win);

	int ch=0;

	do{
		ch=wgetch(win);
		switch(ch){
			case '1':
				break;
			case '2':
				break;
			case '3':
				about_scrn();
				break;
			case '4':
				break;
			default:
				break;
		}
		touchwin(win);
		wrefresh(win);
	}while(ch!='4');

	delwin(win);
}
