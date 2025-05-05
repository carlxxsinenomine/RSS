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
    char courseCode[20];
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
struct Building *createBuilding(int bldgNum);
void printBuildingNumber(WINDOW *win,int width);
struct Building *selectBuilding(WINDOW *win,int height,int width,int bldgNum);
void printRooms();
struct Rooms* createRoom(struct Building *building, int roomNumber);
void printRoomNumber(WINDOW *win,int width,struct Building *building);
struct Rooms* selectRoom(struct Building *building, int roomNumber);
void printSelectedRoom(struct Building *building, struct Rooms* room);
void addSchedule(struct Rooms *room, int dayIndex, const char *courseCode, const char *time);
void upToLower(char word[10]);
void clearBuildingList();

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

    status_bar(win,"User/Buildings");

    const char *bldng[]={
        "____  _   _ ___ _     ____ ___ _   _  ____ ____  ",
        "| __ )| | | |_ _| |   |  _ \\_ _| \\ | |/ ___/ ___| ",
        "|  _ \\| | | || || |   | | | | ||  \\| | |  _\\___ \\ ",
        "| |_) | |_| || || |___| |_| | || |\\  | |_| |___) |",
        "|____/ \\___/|___|_____|____/___|_| \\_|\\____|____/ ",
    };
    int bldng_row_size=sizeof(bldng)/sizeof(bldng[0]);

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

        struct Building *building = createBuilding(bNumber);
        struct Rooms *room = NULL;

        while(fgets(line, sizeof(line), fptr)) {
           
           if (strlen(line) <= 1) continue;// If empty line skip.
        
            // If Room: is present on the string
            if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                sscanf(line, "Room: %d", &currentRoom);
                room = createRoom(building, currentRoom);
                continue;
            }

            int dayIndex;
            char courseCode[21], time[21];

            if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) {// == 3; if 3 values are read
                if (room) {
                    addSchedule(room, dayIndex, courseCode, time);
                }
            }
        }
        fclose(fptr);
    }
    
    int ch=0;

    while(1){
        wborder(win,'|','|','-','-','+','+','+','+');

        for(int i=0;i<bldng_row_size;i++){
            int len=strlen(bldng[i]);
            int tab=(window_width-len)/2;
            mvwprintw(win,i+3,tab,"%s",bldng[i]);
        }

        printBuildingNumber(win,window_width);

        wrefresh(win);

        int buildingChoice=SelectPrompt(win);

        if(buildingChoice!=-1){
            struct Building *selectedBuilding = selectBuilding(win,height,window_width,buildingChoice);
            
            WINDOW *rooms=newwin(height,window_width,0,width/4);
            
            if(!rooms){
                printf("Failed to load screen\n");
                exit(1);
            }

            wborder(rooms,'|','|','-','-','+','+','+','+');

            status_bar(rooms,"User/Buildings/Rooms");
            
            printRoomNumber(rooms,window_width,selectedBuilding);

            wrefresh(rooms);
            
            int roomOfChoice=SelectPrompt(win);

            if(roomOfChoice!=-1){
            }
            else{
                break;
            }
            
        }
        else{
            break;
        
//
       
//
       
        //printf("\nEnter room number to view schedule: ");
        //scanf("%d", &roomOfChoice);
    //
        //struct Rooms* selectedRoom = selectRoom(selectedBuilding, roomOfChoice);
        //printSelectedRoom(selectedBuilding, selectedRoom);
        }

    
        check_winsize(win,height,window_width);
        wclear(win);
    }
    fclose(listOfBuildingsPtr);
    clearBuildingList();
    delwin(win);
}

int SelectPrompt(WINDOW *win){
	noecho();

	int height,width;
	getmaxyx(win,height,width);
	
	WINDOW *sub=newwin(3,width-2,height-5,(width/2)+1);
	
	if(!sub){
		printf("Failed to load screen\n");
        	exit(1);
    }

	keypad(sub,TRUE);

	char user_input[11]={0};
	int i;
	int ch;

	while(1){
		box(sub,0,0);
		mvwprintw(sub,0,2,"[CRTL + X] Cancel");
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
		}
		//Terminates the buffer
		user_input[i]='\0';

		if(user_input && i>0){
			delwin(sub);
			return atoi(user_input);
		}
		else{
			wclear(sub);
		}
	}
	keypad(sub,FALSE);
}

struct Building *createBuilding(int bldgNum){
    struct Building *newBuilding = (struct Building *) malloc(sizeof(struct Building));
    
    if(newBuilding == NULL){
        printf("Memory allocation failed!\n");
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

//date added: 04/15
void printBuildingNumber(WINDOW *win,int width) {
    struct Building *current = bldgHead;
    int i=0;
    while(current != NULL) {
        mvwprintw(win,10+i,(width/2)-6,"Building %d", current->buildingNumber);
        current = current->next;
        i++;
    }
}

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
        }
        current = current->next; // Iterate through the next List
    }
    return NULL;
}

//date edited: 04/15
void printRoomNumber(WINDOW *win,int width,struct Building *building){
    int currentBuildingNumber = building->buildingNumber;
    mvwprintw(win,10,(width/2)-6,"Building %d",currentBuildingNumber);

    struct Rooms *current = building->head; 

    int i=0;
    while(current != NULL) {
        mvwprintw(win,12+i,(width/2)-6,"Room %d", current->roomNumber);
        current = current->next;
        i++;
    }
}

void printRooms() {
    if (head == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    struct Rooms *current = head;
    printf("Room List:\n");
    while (current != NULL) {
        printf("Room %d:\n", current->roomNumber);
        
        //tig ttravese niya ang list
        for (int i = 0; i < current->scheduleCount; i++) {
            printf("  %s, %s at %s\n",             
                   current->schedules[i].day,
                   current->schedules[i].courseCode,
                   current->schedules[i].time);
        }
        current = current->next;
    }
}

void addSchedule(struct Rooms *room, int dayIndex, const char *courseCode, const char *time) {
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
    strcpy(sched->courseCode, courseCode);
    strcpy(sched->time, time);
}

struct Rooms* createRoom(struct Building *_building, int roomNumber) {

    struct Rooms* newRoom = (struct Rooms *) malloc(sizeof(struct Rooms));
    if (!newRoom) { // if newRoom is NULL
        printf("Memory allocation failed.\n");
        return NULL;
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
// Pumili ng room na imomodify e.g. print, or baguhin yung values
struct Rooms* selectRoom(struct Building *building, int roomNumber) {
    struct Rooms* current = building->head;
    while (current != NULL) {
        if (current->roomNumber == roomNumber) { // if val of current->roomNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }

        if (current->next == NULL) {
            printf("Invalid Room");
            return NULL;
        }
        current = current->next; // Iterate thruogh the next List
    }
    return NULL;
}

// iprint ung list ng selected na room
void printSelectedRoom(struct Building *building, struct Rooms* room) {
    int currentRoomNumber = room->roomNumber;
    printf("Room Number: %d\n", currentRoomNumber);
    // printf("Room Sched Count: %d", room->scheduleCount);
    if (building->head == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    for (int i = 0; i < room->scheduleCount; i++) {
            printf("  %s, %s at %s\n",
               room->schedules[i].day,
               room->schedules[i].courseCode,
               room->schedules[i].time);
        }
}

//date added: 04/23
//date edited: 04/24
void upToLower(char word[10]) {
    for(int i = 0; i< strlen(word); i++) word[i] = tolower(word[i]);
}

void clearBuildingList() {
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