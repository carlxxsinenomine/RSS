#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

//Checks window size limit so text don't overlap
void check_winsize(WINDOW *win,int height,int width){
    int temp_h=height,temp_w=width;
    getmaxyx(win,height,width);

    //Calculate height and width nung terminal (mano-mano yan hanggang di mag overlap text)
    if(height<38 || width<105 || height!=temp_h || width!=temp_w){
        endwin();
        printf("Program Crashed.\n");
        printf("Screen is maybe:\n   1. Too small\n   2. Changed\n");
        printf("Note: Do not resize window while program is running. Zoom out or resize terminal window beforehand for program to run properly.\n");
        exit(1);
    }
}

//Auth function
int auth(WINDOW *win,char pass[]){
    noecho();

    int height,width;
    getmaxyx(win,height,width);

    //New win for auth (pangit subwin)
    WINDOW *sub=newwin(3,width-2,height/2,(width/2)+1);
    if(!sub){
        printf("Failed to load screen\n");
        exit(1);
    }

    keypad(sub,TRUE);

    //Pass limit 19 to make space for NUL char
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

            //ASCII value ng backspace 127
            if(ch==KEY_BACKSPACE || ch==127){
                if(i>0){
                    i--;
                    input_pass[i]='\0';
                    mvwprintw(sub,y,x-1," ");
                    wmove(sub,y,x-1);
                    wrefresh(sub);
                }
            }
            //ASCII value ng esc 24
            else if(ch==24){
                delwin(sub);
                return 0;
            }
            //32-126 only kasi yan lang printable ASCII
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

//Status bar function
void status_bar(WINDOW *win,char *status){
    int height,width;
    getmaxyx(win,height,width);

    int len=strlen(status);

    wattrset(win,A_REVERSE);
    mvwprintw(win,height-2,(width-len)-2,"%s",status); //Prints sa bottom right side
    wattrset(win,A_NORMAL);
}