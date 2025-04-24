#include <ncurses.h>

int main(){
	initscr();
	noecho();
	curs_set(0);

	int height,width;
	getmaxyx(stdscr,height,width);

	int window_width=width/2;

	//Set main window sa gitna (same lang sa others di ko alam kung practical to pero mas okay ata na separate allocation ng windows for each)
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
		"[1] User ",
		"[2] Admin",
		"[3] About",
		"[4] Exit "
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
				if(auth(win,"123")){
					user_scr();
				}
				break;
			case '2':
				if(auth(win,"admin")){
					admin_scr();
				}
				else{
					break;
				}
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

        endwin();
        return 0;
}

