#include "ui.h"

//Checks window size limit
void check_winsize(WINDOW *win,int height,int width){
	int temp_h=height,temp_w=width;
	getmaxyx(win,height,width);
	if(height<38 || width<105 || height!=temp_h || width!=temp_w){
		endwin();
		printf("Program Crashed.\n");
		printf("Screen is maybe:\n   1. Too small\n   2. Changed\n");
		printf("Note: Do not resize window while program is running. Zoom out or resize terminal window beforehand for program to run properly.\n");
		exit(1);
	}
}

void auth(WINDOW *win,char pass[]){
	curs_set(1);
	noecho();

	int height,width;
	getmaxyx(win,height,width);

	WINDOW *sub=subwin(win,3,width-2,height/2,(width/2)+1);
	if(!sub){
		printf("Failed to load screen\n");
		exit(1);
	}

	keypad(sub,TRUE);
	box(sub,0,0);

	char input_pass[19]={0};
	int i=0;

	mvwprintw(sub,1,1,"Enter User password: ");

	int ch;

	while((ch=wgetch(sub))!='\n' && i<18){
		if(ch==KEY_BACKSPACE){
			
		}
	}

	delwin(sub);
}

void status_bar(WINDOW *win,char *status){
	int height,width;
	getmaxyx(win,height,width);

	int len=strlen(status);

	wattrset(win,A_REVERSE);
	mvwprintw(win,height-2,(width-len)-2,"%s",status);
	wattrset(win,A_NORMAL);
}

//Generate user screen
void user_scrn(void){
	int height,width;
	getmaxyx(stdscr,height,width);

	int window_width=width/2;

	WINDOW *win=newwin(height,window_width,0,width/4);
	if(!win){
		printf("Failed to load screen\n");
		exit(1);
	}

	wborder(win,'|','|','-','-','+','+','+','+');

	auth(win,"123");

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

//Generate admin screen
void admin_scrn(void){

}

//Generate about screen
void about_scrn(void){
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
		"Authors:",
		"",
		"Margarata, Sean Eric",
		"Muñoz, Carl Johannes",
		"Penetrante, Ethaniel James",
		"Santos, Gebhel Anselm",
		"Zuñiga, Jay Mark"
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

//Generate menu screen
void main_scrn(void){
	noecho();
	curs_set(0);

	int height,width;
	getmaxyx(stdscr,height,width);
	
	int window_width=width/2;

	WINDOW *win=newwin(height,window_width,0,width/4);
	
	check_winsize(win,height,window_width);

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

	int title_row_size=sizeof(title)/sizeof(title[0]);
	for(int i=0;i<title_row_size;i++){
		if(i==0){
			wattrset(win,A_BOLD);
		}
		else if(i==9){
			wattrset(win,A_ITALIC);
		}

		int len=strlen(title[i]);
		int tab=(window_width-len)/2;
		mvwprintw(win,i+5,tab,"%s",title[i]);
	}
	wattrset(win,A_NORMAL);

	const char *menu[]={
		"[1] User   ",
		"[2] Admin  ",
		"[3] About  ",
		"[4] Exit   "
	};

	int menu_row_size=sizeof(menu)/sizeof(menu[0]);
	for(int i=0;i<menu_row_size;i++){
		int len=strlen(menu[i]);
		mvwprintw(win,(height/2)+i*2,(window_width-len)/2,"%s",menu[i]);
	}

	status_bar(win,"Menu");

	int ch=0;

	do{
		ch=wgetch(win);
		switch(ch){
			case '1':
				user_scrn();
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
		check_winsize(win,height,window_width);
	}while(ch!='4');

	delwin(win);
}
