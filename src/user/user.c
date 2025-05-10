#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "global.h"

#define MAXCOL 3
#define MAX_SCHEDULES 20
#define MAX_DAYS 6

struct Schedule {
    char day[10];
    char programCode[20];
    char time[20];
};

struct Rooms {
    int roomNumber;
    int scheduleCount;
    struct Schedule schedules[MAX_SCHEDULES];
    struct Rooms *prev;
    struct Rooms *next;
} *head, *last;

struct Building {
    int buildingNumber;

    struct Rooms *head;
    struct Rooms *last;

    struct Building *next;
    struct Building *prev;
} *bldgHead = NULL, *bldgLast = NULL;

const char* DAYS[MAX_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Function declaration
int SelectPrompt(WINDOW *win);
struct Building *_loadBuilding(int bldgNum);
void printBuildingNumber(WINDOW *win,int width);
struct Building *selectBuilding(WINDOW *win,int height,int width,int bldgNum);
struct Rooms* _loadRoom(struct Building *building, int roomNumber);
void printRoomNumber(WINDOW *win,int width,struct Building *building);
struct Rooms* selectRoom(WINDOW *win,int height,int width,struct Building *building, int roomNumber);
void _loadSched(struct Rooms *room, int dayIndex, const char *programCode, const char *time);
void printSched(WINDOW *win,int height,int width,struct Building *building, struct Rooms* room);
void upToLower(char word[10]);
void clearBuildingList(void);

/* @date_added: 04/15/2025
 * @return_type: void
 * @params: void
 * @description: Makes a new window for user screen
 */
void user_scr(void){
    //Get window size
    int height,width;
    getmaxyx(stdscr,height,width);

    int window_width=width/2;

    WINDOW *win=newwin(height,window_width,0,width/4);
    if(!win){
        printf("Failed to load screen\n");
        exit(1);
    }
    
    FILE *fptr, *listOfBuildingsPtr;           

    int bNumber, maxRooms;
    char line[100], bLine[100];
    int currentRoom = 0; // Room 1, or index zero

    head = NULL;
    last = NULL;

    listOfBuildingsPtr = fopen("buildings/current_changes/listOfBuildings.txt", "rt");

    if (listOfBuildingsPtr == NULL) {
        delwin(win);
    }

    const char *bldng[]={
        "____  _   _ ___ _     ____ ___ _   _  ____ ____  ",
        "| __ )| | | |_ _| |   |  _ \\_ _| \\ | |/ ___/ ___| ",
        "|  _ \\| | | || || |   | | | | ||  \\| | |  _\\___ \\ ",
        "| |_) | |_| || || |___| |_| | || |\\  | |_| |___) |",
        "|____/ \\___/|___|_____|____/___|_| \\_|\\____|____/ ",
    };
    int bldng_row_size=sizeof(bldng)/sizeof(bldng[0]);

    const char *rms[]={
        " ____   ___   ___  __  __ ____   ",
        "|  _ \\ / _ \\ / _ \\|  \\/  / ___| ",
        "| |_) | | | | | | | |\\/| \\___ \\ ",
        "|  _ <| |_| | |_| | |  | |___) |",
        "|_| \\_\\\\___/ \\___/|_|  |_|____/ "
    };
    int rms_row_size=sizeof(rms)/sizeof(rms[0]);

    // while loop na nagreread line by line from the listOfBuildings.txt
    while (fscanf(listOfBuildingsPtr, "%s", bLine) != EOF) {
        char dirC[50] = "buildings/current_changes/";
        strcat(dirC, bLine);
        fptr = fopen(dirC, "rt");
        
        if(fptr == NULL){
            perror("Error handling file");
            continue;
        }

        // fgets rineread nya each line of a text, ung max letters na pede nya maread depends on the size of bytes specified
        // sscanf hinahanap nya sa array ang format na inespecify mo. e.g. "Room: 1", tas format mo "Room: %d". mareread nya ung 1
        fgets(line, sizeof(line), fptr);
        sscanf(line, "Building No: %d", &bNumber);
        fgets(line, sizeof(line), fptr);
        sscanf(line, "Max Rooms: %d", &maxRooms);

        struct Building *building = _loadBuilding(bNumber);
        struct Rooms *room = NULL;

        while(fgets(line, sizeof(line), fptr)) {
           
           if (strlen(line) <= 1) continue;// If empty line skip.
        
            // If Room: is present on the string
            if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                sscanf(line, "Room: %d", &currentRoom);
                room = _loadRoom(building, currentRoom);
                continue;
            }

            int dayIndex;
            char programCode[21], time[21];

            if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, programCode, time) == 3) {// == 3; if 3 values are read
                if (room) {
                    _loadSched(room, dayIndex, programCode, time);
                }
            }
        }
        fclose(fptr);
    }
    
    int ch=0;

    //Building loop
    while(1){
        status_bar(win,"User/Buildings");
        wborder(win,'|','|','-','-','+','+','+','+');

        //Print line by line bldng
        for(int i=0;i<bldng_row_size;i++){
            int len=strlen(bldng[i]);
            int tab=(window_width-len)/2;
            mvwprintw(win,i+3,tab,"%s",bldng[i]);
        }

        //Prints existing building numbers
        printBuildingNumber(win,window_width);

        wrefresh(win);

        //Prompts building selection
        int buildingChoice=SelectPrompt(win);

        //Handles building choice error
        if(buildingChoice!=-1){
            //Receives building selection
            struct Building *selectedBuilding = selectBuilding(win,height,window_width,buildingChoice);
            
            //Handles select building error
            if (!selectedBuilding) {
                break;
            }

            //Loop for rooms
            while(1){
                //Create new window for rooms
                WINDOW *room_win=newwin(height,window_width,0,width/4);

                if(!room_win){
                    printf("Failed to load screen\n");
                    exit(1);
                }

                wborder(room_win,'|','|','-','-','+','+','+','+');

                status_bar(room_win,"User/Buildings/Rooms");

                //Print line by line rms
                for(int i=0;i<rms_row_size;i++){
                    int len=strlen(rms[i]);
                    int tab=(window_width-len)/2;
                    mvwprintw(room_win,i+3,tab,"%s",rms[i]);
                }

                //Print room numbers
                printRoomNumber(room_win,window_width,selectedBuilding);

                wrefresh(room_win);

                //Prompt for room choice
                int roomOfChoice=SelectPrompt(win);

                //Handles room choice error
                if(roomOfChoice!=-1){
                    struct Rooms* selectedRoom = selectRoom(room_win,height,window_width,selectedBuilding,roomOfChoice);
                    if (!selectedRoom) {
                        break;
                    }
                    else{
                        printSched(room_win,height,window_width,selectedBuilding,selectedRoom);
                    }
                }
                else{
                    break;
                }
                wclear(room_win);
            }
            
        }
        else{
            break;
        }
        wclear(win);
    }
    //End of user.c
    fclose(listOfBuildingsPtr);
    clearBuildingList();
    delwin(win);
}

/* @date_added: 05/05/2025
 * @return_type: int
 * @params: win
 * @description: Creates new window for prompt
 */
int SelectPrompt(WINDOW *win){
	noecho();

    //Get win size
	int height,width;
	getmaxyx(win,height,width);
	
    //Creates new window for SelectPrompt
	WINDOW *sub=newwin(3,width-2,height-4,(width/2)+1);
	
	if(!sub){
		printf("Failed to load screen\n");
        	exit(1);
    }

	keypad(sub,TRUE);

    //User input limits to 10 + NUL terminator (means only 10 digits input)
	char user_input[11]={0};
	int i;
	int ch;

	while(1){
        //Set default window style
		box(sub,0,0);
		mvwprintw(sub,0,2,"[CRTL + X] Exit");
		mvwprintw(sub,1,1,"Input: ");

		i=0;

		//While user don't press enter
		while((ch=wgetch(sub))!='\n' && i<11){
			int y,x;
			getyx(sub,y,x);

			//ASCII value of backsapce is 127
			if(ch==KEY_BACKSPACE || ch==127){
				if(i>0){
					i--;
					user_input[i]='\0';
					mvwprintw(sub,y,x-1," ");
					wmove(sub,y,x-1);
					wrefresh(sub);
        			}
			}
			//ASCII value of crtl+x key is 24
			else if(ch==24){
				delwin(sub);
				return -1;
			}
            //If user press space then exit
			else if(ch==' '){
				const char no_space[]="Input should not contain spaces";
				mvwprintw(sub,1,(width-strlen(no_space))/2,"%s",no_space);
				wrefresh(sub);
				napms(2000);
                return -1;
			}
			//Number 0-9 character only
			else if(ch>='0' && ch<='9'){
				if(i<10){
					user_input[i]=ch;
					mvwprintw(sub,y,x,"%c",ch);
					i++;
					wrefresh(sub);
				}
			}
            check_winsize(win,height,width);
		}
		//Terminates the buffer
		user_input[i]='\0';

        //Handles user input
		if(i>0){
			delwin(sub);
			return atoi(user_input);
		}
		else{
			wclear(sub);
		}
	}
	keypad(sub,FALSE);
}

/* @date_added: 05/05/2025
 * @return_type: struct Building
 * @params: bldgNum
 * @description: Load buildings
 */
struct Building *_loadBuilding(int bldgNum){
    struct Building *newBuilding = (struct Building *) malloc(sizeof(struct Building));
    
    if(newBuilding == NULL){
        printf("Memory allocation failed!\n");
        exit(1);
    }

    newBuilding->buildingNumber = bldgNum;
    newBuilding->head = NULL;
    newBuilding->last = NULL;
    newBuilding->next = NULL;
    newBuilding->prev = NULL;

    if (bldgHead == NULL) {
        bldgHead = bldgLast = newBuilding; // set bldgLast equal to newBuilding and bldgHead equal to bldgLast
    } else {
        bldgLast->next = newBuilding; // points to newBuilding
        newBuilding->prev = bldgLast;
        bldgLast = newBuilding;
    }
    return newBuilding;
}

/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: void
 * @params: win, width
 * @description: Prints building numbers
 */
void printBuildingNumber(WINDOW *win,int width) {
    struct Building *current = bldgHead;
    int i=0;
    //Prints to win
    while(current != NULL) {
        char bldng_line_size[20];
        int currentBuildingNumber = current->buildingNumber;
        sprintf(bldng_line_size,"Building %d",currentBuildingNumber);
        mvwprintw(win,10+i,(width-strlen(bldng_line_size))/2,"%s", bldng_line_size);
        current = current->next;
        i++;
    }
}

/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: struct Building
 * @params: win, heightm width, bldgNum
 * @description: Select building
 */
struct Building *selectBuilding(WINDOW *win,int height,int width,int bldgNum) {
    struct Building* current = bldgHead;
    while (current != NULL) {
        if (current->buildingNumber == bldgNum) { // if val of current->buildingNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }

        if (current->next == NULL) {
            const char inbn[]="Invalid Building Number";
            wattrset(win,A_REVERSE);
			mvwprintw(win,height/2,(width-strlen(inbn))/2,"%s",inbn);
            wattrset(win,A_NORMAL);
            wrefresh(win);
            napms(2000);
            return NULL;
        }
        current = current->next; // Iterate through the next List
    }
    return NULL;
}

/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: struct Rooms
 * @params: struct Building, roomNumber
 * @description: Load rooms
 */
struct Rooms* _loadRoom(struct Building *_building, int roomNumber) {

    struct Rooms* newRoom = (struct Rooms *) malloc(sizeof(struct Rooms));
    if (!newRoom) { // if newRoom is NULL
        printf("Memory allocation failed.\n");
        return NULL;
        exit(1);
    }

    newRoom->roomNumber = roomNumber;
    newRoom->scheduleCount = 0;
    newRoom->next = NULL;
    newRoom->prev = NULL;

    if (_building->head == NULL) {
        _building->head = _building->last = newRoom; // set last equal to newRoom and head equal to last
    } else {
        _building->last->next = newRoom; // points to newRoom
        newRoom->prev = last;
        _building->last = newRoom;
    }
    return newRoom;
}

/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: void
 * @params: win, width, struct Building
 * @description: Prints room numbers
 */
void printRoomNumber(WINDOW *win,int width,struct Building *building){
    //Status bar for specific building number
    char cur_bldg_status[50];
    int currentBuildingNumber = building->buildingNumber;
    sprintf(cur_bldg_status,"User/Buildings/Rooms/Building %d",currentBuildingNumber);
    status_bar(win,cur_bldg_status);

    struct Rooms *current = building->head; 

    int i=0;
    
    while(current != NULL) {
        //Print room numbers line by line
        char room_line_size[20];
        int currentRoomNumber = current->roomNumber;
        sprintf(room_line_size,"Room %d",currentRoomNumber);
        mvwprintw(win,10+i,(width-strlen(room_line_size))/2,"%s", room_line_size);
        current = current->next;
        i++;
    }
}

// Pumili ng room na imomodify e.g. print, or baguhin yung values
struct Rooms* selectRoom(WINDOW *win,int height,int width,struct Building *building, int roomNumber) {
    struct Rooms* current = building->head;
    while (current != NULL) {
        if (current->roomNumber == roomNumber) { // if val of current->roomNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }
        if (current->next == NULL) {
            const char inrn[]="Invalid Room Number";
            wattrset(win,A_REVERSE);
			mvwprintw(win,height/2,(width-strlen(inrn))/2,"%s",inrn);
            wattrset(win,A_NORMAL);
            wrefresh(win);
            napms(2000);
            return NULL;
        }
        current = current->next; // Iterate thruogh the next List
    }
    return NULL;
}

/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: void
 * @params: struct Rooms, dayINdex, programCode, time
 * @description: Load schedule
 */
void _loadSched(struct Rooms *room, int dayIndex, const char *programCode, const char *time) {
    if (room->scheduleCount >= MAX_SCHEDULES) {
        printf("Cannot add more schedules to room %d\n", room->roomNumber);
        return;
    }

    if (dayIndex < 0 || dayIndex >= MAX_DAYS) {
        printf("Invalid day index: %d\n", dayIndex);
        return;
    }

    struct Schedule *sched = &room->schedules[room->scheduleCount++];
    strcpy(sched->day, DAYS[dayIndex]);
    strcpy(sched->programCode, programCode);
    strcpy(sched->time, time);
}


/* @date_added: date added: 04/15
 * @ date_edited: 05/05/2025
 * @return_type: void
 * @params: win, height, width, struct Building, struct Rooms
 * @description: Prints building numbers
 */
void printSched(WINDOW *win,int height,int width,struct Building *building, struct Rooms* room) {
    //Window size for schedule
    int sched_height=height/1.2;
    int sched_width=width;

    int sched_y=(height-sched_height)/2;
    int sched_x=(width/2)+1;

    //Generate window size for schedule with start x and start y pos
    WINDOW *sched_win=newwin(sched_height,sched_width-2,sched_y,sched_x);
    if(!sched_win){
		printf("Failed to load screen\n");
		exit(1);
	}

    wborder(sched_win,'|','|','-','-','+','+','+','+');

    char exit[]="[X] Exit";
    mvwprintw(sched_win,sched_height-3,(sched_width-strlen(exit))/2,"%s",exit);

    //Status bar for specific room
    char cur_room_status[70];
    int currentBuildingNumber = building->buildingNumber;
    int currentRoomNumber = room->roomNumber;
    sprintf(cur_room_status,"User/Buildings/Rooms/Building %d/Room %d",currentBuildingNumber,currentRoomNumber);
    status_bar(sched_win,cur_room_status);

    struct Rooms *current = building->head; 
    if (building->head == NULL) {
        return;
    }

    //Header for schedule
    const char *sched_header[]={
        "Day                    Program Code                  Time",
        "--------------------------------------------------------------------"
    };

    //Print line by line of constant sched_header
    int sched_header_size=sizeof(sched_header)/sizeof(sched_header[0]);
	for(int i=0;i<sched_header_size;i++){
		int len=strlen(sched_header[i]);
		mvwprintw(sched_win,4+i,(sched_width-len)/2,"%s",sched_header[i]);
	}

    //Print schedule within room
    for (int i = 0; i < room->scheduleCount; i++) {
            int day_len=strlen(room->schedules[i].day);
            int cc_len=strlen(room->schedules[i].programCode);
            int time_len=strlen(room->schedules[i].time);
            int total_len=day_len+cc_len+time_len;
            mvwprintw(sched_win,6+i,(sched_width-strlen(sched_header[0]))/2,"%-23s%-29s%s",
               room->schedules[i].day,
               room->schedules[i].programCode,
               room->schedules[i].time);
    }

    int ch;

    //Only exits when X is pressed
    while (1) {
        ch = wgetch(sched_win);
        if (toupper(ch) == 'X' || ch==24) {
            break;
        }
        check_winsize(win,height,width/2);
    }
    delwin(sched_win);
}

/* @date_added: date added: 04/23
 * @ date_edited: 05/05/2025
 * @return_type: void
 * @params: word
 * @description: Lower characters within string
 */
void upToLower(char word[10]) {
    for(int i = 0; i< strlen(word); i++) word[i] = tolower(word[i]);
}

/* @date_added: date added: 05/05/2025
 * @return_type: void
 * @params: void
 * @description: Free allocated memory of building
 */
void clearBuildingList(void){
    struct Building *current = bldgHead;
    while (current != NULL) {
        struct Building *next = current->next;
        // Free rooms in the building
        struct Rooms *room = current->head;
        while (room != NULL) {
            struct Rooms *nextRoom = room->next;
            free(room);
            room = nextRoom;
        }
        free(current);
        current = next;
    }
    bldgHead = bldgLast = NULL;
}