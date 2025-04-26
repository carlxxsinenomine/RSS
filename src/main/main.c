#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "screens.h"
#include "global.h"

//Function initialization
int auth(WINDOW *win,char pass[]);

/* @date_added: 04/10/2025
 * @return_type: int
 * @params: argc, argv
 * @description: Main window. Utilizes every function defined.
 */
int main(){
	initscr();
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

	//Calls screen functions
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
				about_scr();
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

/* @date_added: 04/16/2025
 * @return_type: int
 * @params: window, password
 * @description: Creates a window to accept user input for password. Verifies password for program to continue.
 */
int auth(WINDOW *win,char pass[]){
    noecho();

    int height,width;
    getmaxyx(win,height,width);

    WINDOW *sub=newwin(3,width-2,height/2,(width/2)+1);
    if(!sub){
        printf("Failed to load screen\n");
        exit(1);
    }

    keypad(sub,TRUE);

    char input_pass[19]={0};
    const char pass_lim[]="Reached password limit";
    int i;
    int ch;

    while(1){
        box(sub,0,0);
        mvwprintw(sub,0,2,"[CRTL + X] Cancel");
        mvwprintw(sub,1,1,"Enter User password: ");

        i=0;

        //While user don't press enter
        while((ch=wgetch(sub))!='\n' && i<19){
            int y,x;
            getyx(sub,y,x);

            //ASCII value of backsapce is 127
            if(ch==KEY_BACKSPACE || ch==127){
                if(i>0){
                    i--;
                    input_pass[i]='\0';
                    mvwprintw(sub,y,x-1," ");
                    wmove(sub,y,x-1);
                    wrefresh(sub);
                }
            }
            //ASCII value of ESC key is 24
            else if(ch==24){
                delwin(sub);
                return 0;
            }
            //Printable ASCII characters are only within the range of 32-126
            else if(ch>=32 && ch<=126){
                if(i<18){
                    input_pass[i]=ch;
                    mvwprintw(sub,y,x,"%c",ch);
                    i++;
                    wrefresh(sub);
                }
            }
        }

        //Terminates the buffer
        input_pass[i]='\0';

        if(strcmp(input_pass,pass)==0){
            wclear(sub);
            wrefresh(sub);
            delwin(sub);
            return 1;
        }
        else{
            wclear(sub);
        }
    }

    keypad(sub,FALSE);
}

