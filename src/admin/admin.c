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
};

struct Buildings {
    int buildingNumber;
    int maxRooms;
    struct Rooms* head;
    struct Rooms* last;
    struct Buildings *prev;
    struct Buildings *next;
} *bHead=NULL, *bLast=NULL;

char* DAY[MAX_DAYS] = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};

void printBuildings();
void printRooms(struct Buildings *building);
void _loadSched(struct Rooms *room, int dayIndex, const char *courseCode, const char *time);
struct Buildings* _loadBuildings(int buildingNumber, int maxRms);
struct Rooms* _loadRoom(int roomNumber, struct Buildings *currentBuilding);
struct Rooms* selectRoom(int roomNumber, struct Buildings* currentBuilding);
struct Buildings* selectBuilding(int bNum);
void printSelectedRoom(struct Rooms* room);
void _saveCurrentChanges(struct Buildings *current);
void _saveLastChanges(struct Buildings *current);
void editRoomSchedule(struct Rooms *room);
void printLastChanges(struct Buildings *building, struct Rooms *room);
void addRoom(struct Buildings *building);
void addBuilding();
void deleteRoomSchedule(struct Rooms *room);
void deleteRoom(struct Buildings* currBuilding);
void deleteBuilding(struct Buildings* currBuilding, int buildingToDelete);
void upToLower(char word[10]);
void addRoomSchedule(struct Rooms* room);
void sortSchedules(struct Rooms* room);
void editBuilding(struct Buildings *building);
void revertChanges(struct Buildings *current);
void freeAllLists();

/* @date_added: 04/10/2025
 * @return_type: void
 * @params: void
 * @description: Makes a new window for admin screen
 */
void admin_scr(void){
    int height,width;
    getmaxyx(stdscr,height,width);

    int window_width=width/2;

    WINDOW *win=newwin(height,window_width,0,width/4);
    if(!win){
        printf("Failed to load screen\n");
        exit(1);
    }

    FILE *LOBPtr, *CBPtr;  

    int bNumber, maxRooms;
    char line[100];
    char buildingText[100];

    LOBPtr = fopen("./buildings/current_changes/listOfBuildings.txt", "rt");
    if (LOBPtr == NULL) {
        FILE *createFile;
        createFile = fopen("./buildings/current_changes/listOfBuildings.txt", "wt");
        fclose(createFile);
    }
    int isEmpty = 0;
    int bytes_read = fread(line, 1, sizeof(line), LOBPtr); // check if may laman ang file
    if (bytes_read) {
        rewind(LOBPtr); // ibalik sa read mode
        while(fgets(line, sizeof(line), LOBPtr)) {

            // Stores the Building file name
            sscanf(line, "%s", buildingText);
            // printf("Current Building: %s\n", buildingText);
            char dirChanges[50] = "./buildings/current_changes/";
            strcat(dirChanges, buildingText);
            CBPtr = fopen(dirChanges, "rt");
            // fgets rineread nya each line of a text, ung max letters na pede nya maread depends on the size of bytes specified
            // sscanf hinahanap nya sa array ang format na inespecify mo. e.g. "Room: 1", tas format mo "Room: %d". mareread nya ung 1
            fgets(line, sizeof(line), CBPtr);
            sscanf(line, "Building No: %d", &bNumber);
    
            fgets(line, sizeof(line), CBPtr);
            sscanf(line, "Max Rooms: %d", &maxRooms);

            int roomsCount = 0; // for checking purposes;
            struct Buildings *building = _loadBuildings(bNumber, maxRooms);
            struct Rooms *room = NULL;
            while (fgets(line, sizeof(line), CBPtr)) {
                // If Room: is present on the string
                if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                    roomsCount++;
                    if(roomsCount >= building->maxRooms) {
                        // Error sumobra sa maxRoom ung room na nareread from file
                        break;
                    }
                    int currentRoom = 0;
                    sscanf(line, "Room: %d", &currentRoom);
                    room = _loadRoom(currentRoom, building);
                    continue;
                }
    
                if (strlen(line) <= 1)
                    continue; // If empty line skip.
    
                int dayIndex;
                char courseCode[21], time[21];
    
                if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
                    if (room)
                        _loadSched(room, dayIndex, courseCode, time);
            }
            fclose(CBPtr);
        }
        struct Buildings *currentBuilding = bHead;
        while (currentBuilding != NULL) {
            struct Rooms *currentRoom = currentBuilding->head;
            while (currentRoom != NULL) {
                sortSchedules(currentRoom);
                currentRoom = currentRoom->next;
            }
            currentBuilding = currentBuilding->next;
        }
    }

    const char *bldng_nav_text[]={
        "[1] Add Building",
        "[2] Edit Building",
        "[3] Delete Building",
        "[4] View Building",
        "[5] Revert Changes"
    };

    fclose(LOBPtr);
// Sort each schedules per buildings and rooms
    while(1) {
        wborder(win,'|','|','-','-','+','+','+','+');

        status_bar(win,"Admin");

        wrefresh(win);


        int bnt_row_size=sizeof(bldng_nav_text)/sizeof(bldng_nav_text[0]);

        for(int i=0;i<bnt_row_size;i++){
            int len=strlen(bldng_nav_text[i]);
            int tab=(window_width-len)/2;
            mvwprintw(win,i+3,tab,"%s",bldng_nav_text[i]);
        }

        char option;
        printBuildings();

        printf("[1] Add Building:\n[2] Edit Building:\n[3] Delete Building:\n[4] View Building:\n[5] Revert Changes");
        scanf(" %c", &option);
        if(!bytes_read && option != '1') {printf("Buildings unavailable, please add buildings first."); continue;}
        if(option == 'q' || option == 'Q') break; // if shift+x

        if(option == '1') {
            addBuilding();
            continue;
        }

        int bNum;
        struct Buildings* selectedBuilding;
        printf("Select Building: ");
        scanf("%d", &bNum);
        selectedBuilding = selectBuilding(bNum);

        switch(option) {
            case '2':
                editBuilding(selectedBuilding);
                continue;
            case '3':
                deleteBuilding(selectedBuilding, bNum);
                continue;
            case '4':
                printRooms(selectedBuilding);
                break;
            case '5':
                revertChanges(selectedBuilding);
            default:
                continue;
        }

        printf("[1] Add Room:\n[2] Delete Room:\n[3] Edit Room:\n[4] View Room:\n");
        scanf(" %c", &option);
        int roomOfChoice;
        printRooms(selectedBuilding);
        printf("Select Room: ");
        scanf("%d", &roomOfChoice);
        struct Rooms* selectedRoom = selectRoom(roomOfChoice, selectedBuilding);

        switch (option) {
            case '1': // Add Room
                _saveLastChanges(selectedBuilding);
                addRoom(selectedBuilding);
                _saveCurrentChanges(selectedBuilding);
                continue;
            case '2':
                _saveLastChanges(selectedBuilding);
                deleteRoom(selectedBuilding);
                _saveCurrentChanges(selectedBuilding);
                continue;
            case '3':
                _saveLastChanges(selectedBuilding);
                editBuilding(selectedBuilding);
                _saveCurrentChanges(selectedBuilding);
                continue;
            case '4':
                printSelectedRoom(selectedRoom);
                break;
            default:
                break;
        }

        printf("[1] Add Schedule:\n[2] Delete Schedule:\n[3] Edit Schedule:\n[4] Print Last Changes:\n");
        scanf(" %c", &option);
        // kulang pa ng edit
        switch (option) {
            case '1': // Add Room Sched
                _saveLastChanges(selectedBuilding);
                addRoomSchedule(selectedRoom);
                _saveCurrentChanges(selectedBuilding);
                break;
            case '2': // Delete Room Sched
                _saveLastChanges(selectedBuilding);
                deleteRoomSchedule(selectedRoom);
                _saveCurrentChanges(selectedBuilding);
                break;
            case '3': // Edit Room Sched
                _saveLastChanges(selectedBuilding);
                editRoomSchedule(selectedRoom);
                _saveCurrentChanges(selectedBuilding);
                break;
            case '4':
                // pero sa ncurses na dat naka side by side comparison if doable
                printSelectedRoom(selectedRoom);
                printLastChanges(selectedBuilding, selectedRoom);
                printf("[1] Revert Changes\n[2]Back");
                scanf(" %c", &option);
                if(option == '1')
                    revertChanges(selectedBuilding);
                else
                    continue;
                break;
        }
    }
    freeAllLists();
    delwin(win);
}

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

/**
 * @date_added: 4/16
 * @date_edited: 05/09/2025
 * @return_type: void
 * @description: Prints the list of Buildingss
 */
void printBuildings() {
    struct Buildings *current = bHead;
    printf("Building List:\n");
    while (current != NULL) {
        printf("Building %d:\n", current->buildingNumber);
        current = current->next;
    }
}