#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screens.h"
#include "global.h"

//Function initialization
int manual(int argc,char *argv[]);
int auth(WINDOW *win);

/* @date_added: 04/10/2025
 * @return_type: int
 * @params: argc, argv
 * @description: Main window. Utilizes every function defined.
 */
int main(int argc,char *argv[]){
	if(!manual(argc,argv)) {
		return 0;
	};

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
				user_scr();
				break;
			case '2':
				if(auth(win)){
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
/* @date_added: 04/30/2025
 * @params: argc, argv
 * @description: Prints manual
 */
int manual(int argc,char *argv[]){
	const char *Man[]={
		"Manual:",
		"+-----------+---------------------------------------------+-------+",
		"| Main Menu |                 Description                 | Usage |",
		"+-----------+---------------------------------------------+-------+",
		"| User      | View buildings->rooms->schedules            | 1     |",
		"| Admin     | View & edit buildings->rooms->schedules     | 2     |",
		"| About     | Contains info about: RSS, authors, and more | 3     |",
		"| Exit      | Exit button                                 | 4     |",
		"+-----------+---------------------------------------------+-------+",
		" ",
		"Note before proceeding:",
		"- Size of terminal is at least 37X104:",
		"\tResize your window by: zooming out or resizing your terminal window",
		"- src/passwords must contain admin.txt, and users.txt or it will create an empty file. Program will not continue unless there is a text within it.",
		"\tExample: admin123\n\t\t admin256",
		"\nNavigation:",
		"[1] User, [2] Admin, [3] About, [4] Exit",
		"\nUser:",
		"User->Auth->Buildings->Rooms->Schedule",
		"Auth: (Input password from src/passwords), Ctrl+X to exit",
		"Buildings: Type building number",
		"Rooms: Type room number",
		"Schedule: Crtl+X to exit",
		"\nAdmin:",
		"Admin->Auth->Buildings->Rooms->Schedule",
		"Auth: (Input password from src/passwords), Ctrl+X to exit",
		"Buildings:]",
		"",
	};

	if (argc==1) {
		return 1;
	}
	else if(argc==2 && strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0){
		for(int i=0;i<(sizeof(Man)/sizeof(Man[0]));i++) {
			printf("%s\n",Man[i]);
		}
		return 0;
	}
	else{
		printf("Invalid argument\n");
		printf("\nValid commands: -h\t--help\n");
		exit(1);
	}
}

/* @date_added: 04/16/2025
 * @return_type: int
 * @params: window, password
 * @description: Creates a window to accept user input for password. Verifies password for program to continue.
 */
int auth(WINDOW *win){
	noecho();

	int height,width;
	getmaxyx(win,height,width);
	
	WINDOW *sub=newwin(3,width-2,height/2,(width/2)+1);
	
	if(!sub){
		printf("Failed to load screen\n");
        	exit(1);
    	}

	//File name for admin.txt
	const char *Fname;
	Fname="src/passwords/admin.txt";

	keypad(sub,TRUE);

	char input_pass[19]={0};
	int i;
	int ch;

	while(1){
		box(sub,0,0);
		mvwprintw(sub,0,2,"[CRTL + X] Cancel");
		mvwprintw(sub,1,1,"Enter password: ");

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
			//ASCII value of crtl+x key is 24
			else if(ch==24){
				delwin(sub);
				return 0;
			}
			else if(ch==' '){
				const char no_space[]="Password should not contain spaces";
				mvwprintw(sub,1,(width-strlen(no_space))/2,"%s",no_space);
				wrefresh(sub);
				napms(2000);
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

		FILE *pw=fopen(Fname,"rt");
		if(!pw){
			const char cnof[]="Could not open file";
			mvwprintw(sub,1,(width-strlen(cnof))/2,"%s",cnof);
			wrefresh(sub);
			napms(1000);
			
			pw=fopen(Fname,"wt");
			fprintf(pw,"admin");
			fclose(pw);
			const char cefa[]="Created empty file at src/passwords. Default password: admin";
			mvwprintw(sub,1,(width-strlen(cefa))/2,"%s",cefa);
			wrefresh(sub);
			napms(2000);
			return 0;
		}

		int verification=0;
		char file_pass[19];

		while(fgets(file_pass,sizeof(file_pass),pw)!=NULL){
			file_pass[strcspn(file_pass,"\r\n")]='\0';
			if(strcmp(input_pass,file_pass)==0){
				verification=1;
				break;
			}
		}
		
		fclose(pw);

		if(verification){
			delwin(sub);
			return 1;
		}
		else{
			wclear(sub);
		}
	}
	keypad(sub,FALSE);
}