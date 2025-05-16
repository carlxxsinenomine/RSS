#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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

char* WEEK[MAX_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


char *_strSelectPromt(WINDOW *win, char input_text[]);
struct Buildings* _loadBuildings(int buildingNumber, int maxRms);
struct Rooms* _loadRooms(int roomNumber, struct Buildings *currentBuilding);
struct Rooms* _selectRoom(WINDOW *win,int height,int width,int roomNumber, struct Buildings* currentBuilding);
struct Buildings* _selectBuilding(WINDOW *win,int height,int width,int bNum);
void _printBuildings(WINDOW *win,int height,int width,char status[]);
void _printRoom(WINDOW *win,int height,int width,struct Buildings *building,char status[]);
void _loadSchedules(struct Rooms *room, int dayIndex, const char *courseCode, const char *time);
void _printSched(int height,int width,struct Buildings *building, struct Rooms* room,char status[]);
void _printSchedChanges(WINDOW *win,int height,int width,struct Buildings *building, struct Rooms* room);
void _saveCurrentChanges(struct Buildings *current);
void _saveLastChanges(struct Buildings *current);
void _deleteRoom(struct Buildings* currBuilding,int roomToDelete);
void _deleteBuilding(struct Buildings* currBuilding, int buildingToDelete);
void Up2low(char word[10]);
void _sortSchedules(struct Rooms* room);
void _revertChanges(struct Buildings *current);
void freeAllLists();
void updateListOfBuildings(struct Buildings* building);
int _editRoomSchedule(WINDOW *win,int height,int width, struct Rooms *room);
int _deleteRoomSched(WINDOW *win,int height,int width,struct Rooms *room);
int _addRoom(WINDOW *win,int height,int width,struct Buildings *building);
int _addBuiding(WINDOW *win,int height,int width);
int _selectPromt(WINDOW *win,char input_text[]);
int _addRoomSched(WINDOW *win,int height,int width,struct Rooms* room);
int _isValidTimeString(const char *str);
int convertToMinutes(int hour, int minute, int isAM);

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
        return;
    }

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
                    room = _loadRooms(currentRoom, building);
                    continue;
                }
    
                if (strlen(line) <= 1)
                    continue; // If empty line skip.
    
                int dayIndex;
                char courseCode[21], time[21];
    
                if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
                    if (room)
                        _loadSchedules(room, dayIndex, courseCode, time);
            }
            fclose(CBPtr);
        }
        struct Buildings *currentBuilding = bHead;
        while (currentBuilding != NULL) {
            struct Rooms *currentRoom = currentBuilding->head;
            while (currentRoom != NULL) {
                _sortSchedules(currentRoom);
                currentRoom = currentRoom->next;
            }
            currentBuilding = currentBuilding->next;
        }
    }

    const char *admin_text[]={
        "  _    ____  __  __ ___ _   _",
        "  / \\  |  _ \\|  \\/  |_ _| \\ | |",
        " / _ \\ | | | | |\\/| || ||  \\| |",
        " / ___ \\| |_| | |  | || || |\\  |",
        "/_/   \\_\\____/|_|  |_|___|_| \\_|"
    };
    int adt_row_size=sizeof(admin_text)/sizeof(admin_text[0]);

    const char *bldng_nav_text[]={
        "[1] Add Building   ",
        "[2] Delete Building",
        "[3] View Building  ",
        "[4] Revert Changes ",
    };
    int bnt_row_size=sizeof(bldng_nav_text)/sizeof(bldng_nav_text[0]);

    const char *room_nav_text[]={
        "[1] Add Room   ",
        "[2] Delete Room",
        "[3] View Room  ",
    };
    int rnt_row_size=sizeof(room_nav_text)/sizeof(room_nav_text[0]);

    const char *sched_nav_text[]={
        "[1] Add Schedule      ",
        "[2] Delete Schedule   ",
        "[3] Print Schedule    ",
        "[4] Print Last Changes",
        "[5] Edit Schedule     ",
    };
    int snt_row_size=sizeof(sched_nav_text)/sizeof(sched_nav_text[0]);

    char ext[]="[X] Exit";

    fclose(LOBPtr);
    int flag=0;
    struct Buildings* selectedBuilding=NULL;
    struct Rooms* selectedRoom=NULL;
    struct Buildings* temp=NULL;

// Sort each schedules per buildings and rooms
    while(flag!=-1) {
        int bNum;
        int buildingChoice;
        int roomOfChoice;
        int input;
        wclear(win);
        status_bar(win,"Admin");
        wborder(win,'|','|','-','-','+','+','+','+');

        for(int i=0;i<adt_row_size;i++){
            int len=strlen(admin_text[i]);
            int tab=(window_width-len)/2;
            mvwprintw(win,i+3,tab,"%s",admin_text[i]);
        }

        for(int i=0;i<bnt_row_size;i++){
            int len=strlen(bldng_nav_text[i]);
            int tab=(window_width-len)/2;
            mvwprintw(win,(height/2)-bnt_row_size+i*2,tab,"%s",bldng_nav_text[i]);
        }

        mvwprintw(win,height-4,(window_width-strlen(ext))/2,"%s",ext);

        wrefresh(win);

        // input=wgetch(win);
        if(flag==0){
            input=wgetch(win);
            if(!bytes_read && input!='1'){
                const char no_bldng_avail[]="Buildings unavailable, please add buildings first";
                wattrset(win,A_REVERSE);
				mvwprintw(win,height/2,(width-strlen(no_bldng_avail))/2,"%-20s",no_bldng_avail);
                wattrset(win,A_NORMAL);
				wrefresh(win);
				napms(2000);
                admin_scr();
            }

            switch(input) {
                case '1':
                    _printBuildings(win, height, window_width, "Admin/Add Building");
                    int addBldng = _addBuiding(win, height, window_width);
                    bytes_read = 1;
                    break;
                case '2':
                    _printBuildings(win, height, window_width, "Admin/Delete Building");
                    int delBldng = _selectPromt(win, "Input Building Number to Delete: ");
                    if (delBldng == -1) continue;
                    selectedBuilding = _selectBuilding(win, height, window_width, delBldng);
                    if (!selectedBuilding)
                        break;
                    _deleteBuilding(selectedBuilding, delBldng);
                    // flag=0;
                    break;
                case '3':
                    _printBuildings(win, height, window_width, "Admin/View Building");
                    buildingChoice = _selectPromt(win, "Input Building: ");
                    if (buildingChoice != -1) {
                        selectedBuilding = _selectBuilding(win, height, window_width, buildingChoice);
                        if (selectedBuilding)
                            flag = 1;
                    }
                    break;
                case '4':
                    _printBuildings(win, height, window_width, "Admin/Revert Changes");
                    buildingChoice = _selectPromt(win, "Input Building to Revert: ");
                    if (buildingChoice == -1)
                        break;
                    selectedBuilding = _selectBuilding(win, height, window_width, buildingChoice);
                    if (!selectedBuilding)
                        break;
                    _revertChanges(selectedBuilding);
                    // flag=0;
                    break;
                case 'x':
                case 'X':
                    flag=-1;
                    freeAllLists();
                    delwin(win);
                    continue;
                }
        }
        else if(flag==1){
            wclear(win);
            status_bar(win,"Admin/View Building/Rooms");
            wborder(win,'|','|','-','-','+','+','+','+');

            for(int i=0;i<adt_row_size;i++){
                int len=strlen(admin_text[i]);
                int tab=(window_width-len)/2;
                mvwprintw(win,i+3,tab,"%s",admin_text[i]);
            }

            for(int i=0;i<rnt_row_size;i++){
                int len=strlen(room_nav_text[i]);
                int tab=(window_width-len)/2;
                mvwprintw(win,(height/2)-rnt_row_size+i*2,tab,"%s",room_nav_text[i]);
            }

            mvwprintw(win,height-4,(window_width-strlen(ext))/2,"%s",ext);

            wrefresh(win);

            input=wgetch(win);

            switch (input) {
                case '1':
                    _printRoom(win, height, window_width, selectedBuilding, "Admin/View Building/Add Rooms");
                    int addRoom = _addRoom(win, height, window_width, selectedBuilding);
                    break;
                case '2':
                    _printRoom(win, height, window_width, selectedBuilding, "Admin/View Building/Delete Rooms");
                    roomOfChoice = _selectPromt(win, "Select Room to Delete: ");
                    if (roomOfChoice == -1) 
                        break;
                    selectedRoom = _selectRoom(win, height, window_width, roomOfChoice, selectedBuilding);
                    if (!selectedRoom)
                        break;
                    _deleteRoom(selectedBuilding, roomOfChoice);
                    break;
                case '3':
                    char bldng_line_size[40];
                    sprintf(bldng_line_size, "Admin/View Building/Building %d/Rooms", selectedBuilding->buildingNumber);
                    status_bar(win, bldng_line_size);
                    _printRoom(win, height, window_width, selectedBuilding, "Admin/View Room");
                    roomOfChoice = _selectPromt(win, "Input Room: ");
                    if (roomOfChoice != -1) {
                        selectedRoom = _selectRoom(win, height, window_width, roomOfChoice, selectedBuilding);
                        if (selectedRoom)
                            flag = 2;
                    }
                    break;
                case 'x':
                case 'X':
                    flag=0;
                    selectedBuilding=NULL;
                    break;
                }                
        }
        else if(flag==2) {
            wclear(win);
            status_bar(win,"Admin/Buildigs/Rooms/Schedules");
            wborder(win,'|','|','-','-','+','+','+','+');

            for (int i = 0; i < adt_row_size; i++) {
                int len = strlen(admin_text[i]);
                int tab = (window_width - len) / 2;
                mvwprintw(win, i + 3, tab, "%s", admin_text[i]);
            }

            for (int i = 0; i < snt_row_size; i++) {
                int len = strlen(sched_nav_text[i]);
                int tab = (window_width - len) / 2;
                mvwprintw(win, (height / 2) - rnt_row_size + i * 2, tab, "%s", sched_nav_text[i]);
            }

            mvwprintw(win, height - 4, (window_width - strlen(ext)) / 2, "%s", ext);
            wrefresh(win);

            input=wgetch(win);

            switch (input) {
                case '1':
                    temp = selectedBuilding; // Store the current selectedBuilding
                    _printSched(height, window_width, selectedBuilding, selectedRoom, "Add Sched: ");
                    int addRoomSched = _addRoomSched(win, height, window_width, selectedRoom);
                    if (addRoomSched == -1) {
                        continue;
                    }
                    _saveLastChanges(temp); // Save the current contents before changes
                    _saveCurrentChanges(selectedBuilding);
                    break;
                case '2':
                    _printSched(height, window_width, selectedBuilding, selectedRoom, "Add Sched");
                    temp = selectedBuilding;
                    int isDeleted=_deleteRoomSched(win, height, window_width, selectedRoom);
                    if(!isDeleted) break;
                    _saveLastChanges(temp);
                    _saveCurrentChanges(selectedBuilding);
                    break;
                case '3':
                    _printSched(height, window_width, selectedBuilding, selectedRoom, "Add Sched");
                    wgetch(win);
                    break;
                case '4':
                    _printSchedChanges(win, height, window_width, selectedBuilding, selectedRoom);
                    break;
                case '5':
                    _printSched(height, window_width, selectedBuilding, selectedRoom, "Edit Sched");
                    temp = selectedBuilding;
                    int isEditted=_editRoomSchedule(win, height, window_width, selectedRoom);
                    if(isEditted==-1) break;
                    _saveLastChanges(temp);
                    _saveCurrentChanges(selectedBuilding);
                    break;
                case 'x':
                case 'X':
                    flag=1;
                    selectedRoom=NULL;
                    break;
                }
            }
            wrefresh(win);
            check_winsize(win, height, window_width);
        }
}

int _selectPromt(WINDOW *win,char input_text[]){
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
		mvwprintw(sub,1,1,"%s",input_text);

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

char *_strSelectPromt(WINDOW *win, char input_text[]){
    noecho();

	//Get window size
	int height,width;
	getmaxyx(win,height,width);
	
	//Creates screen for _strSelectPromt
	WINDOW *sub=newwin(3,width-2,height-4,(width/2)+1);
	
	if(!sub){
		printf("Failed to load screen\n");
        exit(1);
    }

	keypad(sub,TRUE);

	char* user_input = malloc(20);
    if (!user_input) {
        delwin(sub);
        return NULL;
    }

	int i;
	int ch;

	//Loop for password
	while(1){
		box(sub,0,0);
		mvwprintw(sub,0,2,"[CRTL + X] Cancel");
		mvwprintw(sub,1,1,"%s",input_text);

		i=0;

		//While user don't press enter and i does not exceed 19 (19 ensures that the user can input upto 19)
		while((ch=wgetch(sub))!='\n' && i<19){
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
                free(user_input);
				delwin(sub);
				return NULL;
			}
			//If user press space then exit
			else if(ch==' '){
				const char no_space[]="Input should not contain spaces";
				mvwprintw(sub,1,(width-strlen(no_space))/2,"%s",no_space);
				wrefresh(sub);
				napms(2000);
				return NULL;
			}
			//Printable ASCII characters are only within the range of 32-126
			else if(ch>=32 && ch<=126){
				//Handles when i<18 only prints when characters at input_pass buffer is i<18
				if(i<18){
					user_input[i]=ch;
					mvwprintw(sub,y,x,"%c",ch);
					i++;
					wrefresh(sub);
				}
			}
		}
		//Terminates the buffer
		user_input[i]='\0';

		if(i>0){
			delwin(sub);
			return user_input;
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

void _printBuildings(WINDOW *win,int height,int width,char status[]) {
    wclear(win);

    wborder(win,'|','|','-','-','+','+','+','+');

    const char *bldng[]={
        "____  _   _ ___ _     ____ ___ _   _  ____ ____  ",
        "| __ )| | | |_ _| |   |  _ \\_ _| \\ | |/ ___/ ___| ",
        "|  _ \\| | | || || |   | | | | ||  \\| | |  _\\___ \\ ",
        "| |_) | |_| || || |___| |_| | || |\\  | |_| |___) |",
        "|____/ \\___/|___|_____|____/___|_| \\_|\\____|____/ ",
    };
    int bldng_row_size=sizeof(bldng)/sizeof(bldng[0]);

    status_bar(win,status);

    for(int i=0;i<bldng_row_size;i++){
        int len=strlen(bldng[i]);
        int tab=(width-len)/2;
        mvwprintw(win,i+3,tab,"%s",bldng[i]);
    }
    struct Buildings *current = bHead;
    int i=0;

    while (current != NULL) {
        char bldng_line_size[20];
        int currentBuildingNumber = current->buildingNumber;
        sprintf(bldng_line_size,"Building %d",currentBuildingNumber);
        mvwprintw(win,10+i,(width-strlen(bldng_line_size))/2,"%s", bldng_line_size);
        current = current->next;
        i++;
    }

    wrefresh(win);
}

/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: AprogCodeepts an Building type Structure
 * @description: Prints the list of Rooms
 */
void _printRoom(WINDOW *win,int height,int width,struct Buildings *building,char status[]) {
    wclear(win);

    wborder(win,'|','|','-','-','+','+','+','+');

    const char *rms[]={
        " ____   ___   ___  __  __ ____   ",
        "|  _ \\ / _ \\ / _ \\|  \\/  / ___| ",
        "| |_) | | | | | | | |\\/| \\___ \\ ",
        "|  _ <| |_| | |_| | |  | |___) |",
        "|_| \\_\\\\___/ \\___/|_|  |_|____/ "
    };
    int rms_row_size=sizeof(rms)/sizeof(rms[0]);

    //Print line by line rms
    for(int i=0;i<rms_row_size;i++){
        int len=strlen(rms[i]);
        int tab=(width-len)/2;
        mvwprintw(win,i+3,tab,"%s",rms[i]);
    }

    //Status bar for specific building number
    char cur_bldg_status[50];
    int currentBuildingNumber = building->buildingNumber;
    sprintf(cur_bldg_status,"%s/Building %d/Rooms",status,currentBuildingNumber);
    status_bar(win,cur_bldg_status);

    struct Rooms *current = building->head;
    int i=0;

    while (current != NULL) {
        //Print room numbers line by line
        char room_line_size[20];
        int currentRoomNumber = current->roomNumber;
        sprintf(room_line_size,"Room %d",currentRoomNumber);
        mvwprintw(win,10+i,(width-strlen(room_line_size))/2,"%s", room_line_size);
        i++;
        current = current->next;
    }

    wrefresh(win);
}

/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: AprogCodeepts a Room type Structure, Int, Char 
 * @description: Adds data to an Array of Schedule Structure of the cuurent Room object passed as an argument
 */
void _loadSchedules(struct Rooms *room, int dayIndex, const char *courseCode, const char *time) {
    if (room->scheduleCount >= MAX_SCHEDULES) {
        printf("Cannot add more schedules to room %d\n", room->roomNumber);
        return;
    }

    if (dayIndex < 0 || dayIndex >= MAX_DAYS) {
        printf("Invalid day index: %d\n", dayIndex);
        return;
    }

    struct Schedule *sched = &room->schedules[room->scheduleCount++];
    strcpy(sched->day, WEEK[dayIndex]);
    strcpy(sched->programCode, courseCode);
    strcpy(sched->time, time);
}

/**
 * @date_added: 4/16
 * @return_type: Buildings Structure
 * @parameter: AprogCodeepts an Int
 * @description: Creates a linked list of Building
 */
struct Buildings* _loadBuildings(int buildingNumber, int maxRms) {
    struct Buildings* newBuilding = (struct Buildings *) malloc(sizeof(struct Buildings));
    if (!newBuilding) { // if new building is NULL
        printf("Memory allocation failed.");
        return NULL;
    }

    newBuilding->buildingNumber = buildingNumber;
    newBuilding->maxRooms = maxRms;
    newBuilding->head = NULL;
    newBuilding->last = NULL;
    newBuilding->next = NULL;
    newBuilding->prev = NULL;

    if (bHead == NULL) {
        bHead = bLast = newBuilding; // set last equal to newRoom and head equal to last
    } else {
        bLast->next = newBuilding; // points to newRoom
        newBuilding->prev = bLast;
        bLast = newBuilding;
    }
    return newBuilding;
    
}
/**
 * @date_added: 4/15
 * @return_type: RoomsStructure
 * @parameter: AprogCodeepts an INt, Buildings Structure
 * @description: Creates a linked list of Rooms inside a structure of Building
 */
struct Rooms* _loadRooms(int roomNumber, struct Buildings *currentBuilding) {

    struct Rooms* newRoom = (struct Rooms *) malloc(sizeof(struct Rooms));

    if (!newRoom) { // if newRoom is NULL
        printf("Memory allocation failed.\n");
        return NULL;
    }

    newRoom->roomNumber = roomNumber;
    newRoom->scheduleCount = 0;
    newRoom->next = NULL;
    newRoom->prev = NULL;

    if (currentBuilding->head == NULL) {
        currentBuilding->head = currentBuilding->last = newRoom; // set last equal to newRoom and head equal to last
    } else {
        currentBuilding->last->next = newRoom; // points to newRoom
        newRoom->prev = currentBuilding->last;
        currentBuilding->last = newRoom;
    }
    return currentBuilding->last;
}

/**
 * @date_added: 4/15
 * @return_type: Rooms Structure
 * @parameter: AprogCodeepts an INt, Buildings Structure
 * @description: returns a Rooms Structure selected by the user for purposes such as Printing, Updating, Deleting of Room
 */
struct Rooms* _selectRoom(WINDOW *win,int height,int width,int roomNumber,struct Buildings* currentBuilding) {
    struct Rooms* current = currentBuilding->head;
    while (current != NULL) {
        if (current->roomNumber == roomNumber) { // if val of current->roomNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }
        if (current->next==NULL){
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
/**
 * @date_added: 4/16
 * @return_type: Buildings Structure
 * @parameter: AprogCodeepts an INt
 * @description: returns a Buildings Structure selected by the user to be able to aprogCodeess Rooms Objects of that Buildings Structure
 */
struct Buildings* _selectBuilding(WINDOW *win,int height,int width,int bNum) {
    struct Buildings* current = bHead;
    while (current != NULL) {
        if (current->buildingNumber == bNum) { // if val of current->roomNumber is equal to current edi same building
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
        current = current->next; // Iterate thruogh the next List
    }
    return NULL;
}

/**
 * @date_added: 05/10
 * @return_type: void
 * @parameter: win, height, width, struct Buildings, struct Rooms, status
 * @description: Prints schedule to screen
 */
void _printSched(int height,int width,struct Buildings *building, struct Rooms* room,char status[]){
    //Window size for schedule
    int sched_height=height/1.2;
    int sched_width=width-2;

    int sched_y=(height-sched_height)/2;
    int sched_x=(width/2)+1;

    //Generate window size for schedule with start x and start y pos
    WINDOW *sched_win=newwin(sched_height,sched_width,sched_y,sched_x);
    if(!sched_win){
		printf("Failed to load screen\n");
		exit(1);
	}

    wborder(sched_win,'|','|','-','-','+','+','+','+');

    //Status bar for specific room
    char cur_room_status[70];
    int currentBuildingNumber = building->buildingNumber;
    int currentRoomNumber = room->roomNumber;
    sprintf(cur_room_status,"Admin/View Building/%s/Building %d/Room %d",status,currentBuildingNumber,currentRoomNumber);
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
            int progCode_len=strlen(room->schedules[i].programCode);
            int time_len=strlen(room->schedules[i].time);
            int total_len=day_len+progCode_len+time_len;
            mvwprintw(sched_win,6+i,(sched_width-strlen(sched_header[0]))/2,"%d. %-23s%-29s%s", i+1,
               room->schedules[i].day,
               room->schedules[i].programCode,
               room->schedules[i].time);
    }
    wrefresh(sched_win);
}

void updateListOfBuildings(struct Buildings* building) {
    int currentBuildingNumber = building->buildingNumber;
    char PATH[100] = "./buildings/current_changes/bld";
    snprintf(PATH,155,"%s%d.txt",PATH,currentBuildingNumber);

    FILE *outFile = fopen("./buildings/current_changes/listOfBuildings.txt", "w");
    if(outFile == NULL) {
        printf("Error reopening file for writing\n");
        exit(1);
    }

    struct Buildings* current = bHead;
    while(current != NULL) {
        fprintf(outFile, "bld%d.txt\n", current->buildingNumber);
        current = current->next;
    }

    fclose(outFile);

    if (remove(PATH) == 0)
        printf("File deleted suprogCodeessfully.\n");
    else
        printf("Unable to delete the file.\n");
}

/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: AprogCodeepts Buildings Structure, And Rooms Structure
 * @description: The Structure Arguments are the structures acquired from the selectBuilding and selectRoom functions, the printSelectedRoom
 * function prints the data contents of an specific room.
 */
void _printSchedChanges(WINDOW *win,int height,int width,struct Buildings *building, struct Rooms* room) {
    int currentRoomNumber = room->roomNumber;
    int currentBuildingNumber = building->buildingNumber;

     //Window size for schedule
     int sched_height=height/1.2;
     int sched_width=(width-1)/2;
 
     int sched_y=(height-sched_height)/2;
 
     //Generate window size for schedule with start x and start y pos
     WINDOW *cur_sched=newwin(sched_height,sched_width,sched_y,width-sched_width);
     WINDOW *last_sched=newwin(sched_height,sched_width,sched_y,width-1);

     if(!cur_sched || !last_sched){
         printf("Failed to load screen\n");
         exit(1);
     }
     
     wborder(cur_sched,'|','|','-','-','+','+','+','+');
     wborder(last_sched,'|','|','-','-','+','+','+','+');
 

     //Status bar for specific room
    char cur_status[70],last_status[70];

    sprintf(cur_status,"Current Schedule: Building %d/Room %d",currentBuildingNumber,currentRoomNumber);
    sprintf(last_status,"Last Changes: Building %d/Room %d",currentBuildingNumber,currentRoomNumber);
    
    status_bar(cur_sched,cur_status);
    status_bar(last_sched,last_status);

    const char *sched_header[]={
        "Day       Program Code           Time",
        "---------------------------------------------"
    };
    int sched_header_size=sizeof(sched_header)/sizeof(sched_header[0]);

	for(int i=0;i<sched_header_size;i++){
		int len=strlen(sched_header[i]);
		mvwprintw(cur_sched,4+i,(sched_width-len)/2,"%s",sched_header[i]);
        mvwprintw(last_sched,4+i,(sched_width-len)/2,"%s",sched_header[i]);
	}

    for (int i = 0; i < room->scheduleCount; i++) {
        int day_len=strlen(room->schedules[i].day);
        int progCode_len=strlen(room->schedules[i].programCode);
        int time_len=strlen(room->schedules[i].time);
        int total_len=day_len+progCode_len+time_len;
        mvwprintw(cur_sched,6+i,(sched_width-strlen(sched_header[0]))/2,"%-10s%-23s%s",
           room->schedules[i].day,
           room->schedules[i].programCode,
           room->schedules[i].time);
    }

    FILE *lastChanges, *temp;
    char strBuildingNumber[5];
    int buildingNumber = building->buildingNumber;
    sprintf(strBuildingNumber, "%d", buildingNumber); // convert into str
    char lastChangesDir[125] = "./buildings/last_changes/last_changes_bld";
    strcat(lastChangesDir, strBuildingNumber);
    strcat(lastChangesDir, ".txt");

    lastChanges = fopen(lastChangesDir, "rt");

    char line[100];
   
    int currentRoom = 0;
    while(fgets(line, sizeof(line), lastChanges)) {
        if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
            sscanf(line, "Room: %d", &currentRoom);
            if(currentRoom == room->roomNumber) {
                while(fgets(line, sizeof(line), lastChanges)) {
                    int i=0;
                    if (strstr(line, "Room:")) {
                        break;
                    }
                    if (strlen(line)>1){
                        int dayIndex;
                        char courseCode[21], time[21];
                        if(sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3){
                            mvwprintw(last_sched,6+i,(sched_width-strlen(sched_header[0]))/2,"%-10s%-23s%s",
                            WEEK[dayIndex],
                            courseCode,
                            time);
                        }
                    }
                }   
                break;
           }
        }
    }

    wrefresh(cur_sched);
    wrefresh(last_sched);

    int ch;

    //Only exits when X is pressed
    while (1) {
        ch = wgetch(cur_sched);
        if (toupper(ch) == 'X' || ch==24) {
            break;
        }
        check_winsize(win,height,width);
    }
    delwin(cur_sched);
    delwin(last_sched);
}

int _editRoomSchedule(WINDOW *win,int height,int width, struct Rooms *room) {
    int rowToEdit=_selectPromt(win, "Select Row to Edit: ");

    if(rowToEdit==-1){
        return -1;
    }
  
    //Fix index
    rowToEdit--;

    struct Schedule *current = &room->schedules[rowToEdit];
    char* roomDay = current->day;
    char* progCode = current->programCode;
    char* roomTime = current->time;

    int len = strlen(roomDay) + strlen(progCode) + strlen(roomTime);
    // New Window
    int _height=height/10;
    int _width=(width-len)/3;

    int _y=(height)/2;
    int _x=width-_width/2;
    WINDOW* _sub_win=newwin(_height, _width, _y, _x);
    if(!_sub_win) {
        printf("Failed to load screen");
        exit(1);
    }

    wborder(_sub_win,'|','|','-','-','+','+','+','+');

    char cur_room_status[70];
    int currentRoomNumber = room->roomNumber;
    sprintf(cur_room_status, "Room: %d", currentRoomNumber);
    status_bar(_sub_win, cur_room_status);

    mvwprintw(_sub_win,_height/2,(_width-len)/2,"%s %s %s",roomDay, progCode, roomTime);
    wrefresh(_sub_win);
    // wattrset(win,A_REVERSE);
    // mvwprintw(win,height/2,(width-len)/2,"%s %s %s",roomDay, progCode, roomTime);
    // wattrset(win,A_NORMAL);
    // wrefresh(win);

    //printf("%s, %s, %s\n", current->day, current->programCode, current->time);
    int option=_selectPromt(win, "What do you want to edit: [1] Day, [2] Course Code, [3] Time, [4] All: ");
    if(option == -1) return -1;
    if(option < 1) option = 1;
    else if(option > 4) option = 4;
    char* day, *programCode, *time;
    switch(option) {
        case 1:
            day=_strSelectPromt(win, "Enter Day: ");
            strcpy(current->day, day);
            free(day);
            break;
        case 2:
            programCode=_strSelectPromt(win, "Enter Program Code: ");
            strcpy(current->programCode, programCode);
            free(programCode);
            break;
        case 3:
            time=_strSelectPromt(win, "Enter Time: ");
            strcpy(current->time, time);
            free(time);
            break;
        case 4:
            day=_strSelectPromt(win, "Enter Day: ");
            programCode=_strSelectPromt(win, "Enter Course Code: ");
            time=_strSelectPromt(win, "Enter Time: ");
            strcpy(current->day, day);
            strcpy(current->programCode, programCode);
            strcpy(current->time, time);
            free(day);
            free(programCode);
            free(time);
            break;
    }
    _sortSchedules(room);
}

// Save updated version
void _saveCurrentChanges(struct Buildings *current) {
    FILE* savePTR;
    char dirCurrent[125] = "./buildings/current_changes/bld";
    sprintf(dirCurrent, "%s%d.txt", dirCurrent, current->buildingNumber);


    savePTR = fopen(dirCurrent, "wt");
    if (!savePTR) {
        return; // Handle file opening error
    }

    fprintf(savePTR, "Building No: %d\n", current->buildingNumber);
    fprintf(savePTR, "Max Rooms: %d\n", current->maxRooms);
    
    struct Rooms *room = current->head;
    while(room != NULL) {
        fprintf(savePTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {  // Fixed: Iterate through all schedules
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, WEEK[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }
            fprintf(savePTR, "%d, %s, %s\n", dayIndex, 
                   room->schedules[i].programCode,  // Fixed: Use current index
                   room->schedules[i].time);       // Fixed: Use current index
        }
        fprintf(savePTR, "\n");
        room = room->next;
    }
    fclose(savePTR);
}

void _saveLastChanges(struct Buildings *current) {
    FILE* changesPTR;
    int buildingNumber = current->buildingNumber;
    int maxRooms = current->maxRooms;
    char dirChanges[125] = "./buildings/last_changes/last_changes_bld";
    sprintf(dirChanges, "%s%d.txt", dirChanges, buildingNumber);

    changesPTR = fopen(dirChanges, "wt");
    if (!changesPTR) {
        return;
    }

    fprintf(changesPTR, "Building No: %d\n", buildingNumber);
    fprintf(changesPTR, "Max Rooms: %d\n", maxRooms);

    struct Rooms *room = current->head;
    while(room != NULL) {
        fprintf(changesPTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {  // Fixed: Iterate through all schedules
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, WEEK[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }
            fprintf(changesPTR, "%d, %s, %s\n", dayIndex, 
                   room->schedules[i].programCode,  // Fixed: Use current index
                   room->schedules[i].time);       // Fixed: Use current index
        }
        fprintf(changesPTR, "\n");
        room = room->next;
    }
    fclose(changesPTR);
}

void _revertChanges(struct Buildings *current) {
    if (!current) return;

    // Save current state as last changes first (so we can undo the revert if needed)
    _saveLastChanges(current);
    int buildingNumber = current->buildingNumber;
    char dirChanges[125] = "./buildings/last_changes/last_changes_bld";
    sprintf(dirChanges, "%s%d.txt", dirChanges, buildingNumber);


    FILE *revertPtr = fopen(dirChanges, "rt");
    if (!revertPtr) {
        // Handle error - file doesn't exist or can't be opened
        return;
    }

    // Clear existing rooms
    struct Rooms *room = current->head;
    while (room != NULL) {
        struct Rooms *nextRoom = room->next;
        free(room);
        room = nextRoom;
    }
    current->head = current->last = NULL;

    char line[100];
    struct Rooms *currentRm = NULL;
    
    // Read building info
    if (fgets(line, sizeof(line), revertPtr)) {
        sscanf(line, "Building No: %d", &current->buildingNumber);
    }
    if (fgets(line, sizeof(line), revertPtr)) {
        sscanf(line, "Max Rooms: %d", &current->maxRooms);
    }

    // Read rooms and schedules
    while (fgets(line, sizeof(line), revertPtr)) {
        if (strstr(line, "Room:")) {
            int currentRoom = 0;
            sscanf(line, "Room: %d", &currentRoom);
            currentRm = _loadRooms(currentRoom, current);
            continue;
        }

        if (strlen(line) <= 1) continue;

        int dayIndex;
        char courseCode[21], time[21];
        if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) {
            if (currentRm) {
                if (currentRm->scheduleCount < MAX_SCHEDULES) {
                    _loadSchedules(currentRm, dayIndex, courseCode, time);
                }
            }
        }
    }

    fclose(revertPtr);
    
    // Sort schedules after loading
    struct Rooms *rm = current->head;
    while (rm != NULL) {
        _sortSchedules(rm);
        rm = rm->next;
    }

    // Save the reverted state as current
    _saveCurrentChanges(current);
}

int _addRoom(WINDOW *win, int height, int width, struct Buildings *building) {
    if (!building) {
        return -1;
    }

    // Check if we can add more rooms
    int roomCount = 0;
    struct Rooms* currentRoom = building->head;
    while (currentRoom != NULL) {
        roomCount++;
        currentRoom = currentRoom->next;
    }

    if (roomCount >= building->maxRooms) {
        const char max_rooms[] = "Maximum rooms reached for this building";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height/2, (width-strlen(max_rooms))/2, "%s", max_rooms);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }

    // Get room number from user
    int roomNumber = _selectPromt(win, "Input Room Number: ");
    if (roomNumber < 1) {
        return -1;
    }

    // check if sumobra sa max room ang inputted room no.
    if(roomNumber%100 > building->maxRooms) {
        const char exists[] = "Room Number should be less than Max Room capacity";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(exists)) / 2, "%s", exists);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }

    // Check if room number already exists
    currentRoom = building->head;
    while (currentRoom != NULL) {
        if (currentRoom->roomNumber == roomNumber) {
            const char exists[] = "Room already exists";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height/2, (width-strlen(exists))/2, "%s", exists);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            return -1;
        }
        currentRoom = currentRoom->next;
    }

    // Create new room
    struct Rooms *newRoom = (struct Rooms *)malloc(sizeof(struct Rooms));
    if (!newRoom) {
        const char no_mem[] = "Memory allocation failed";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height/2, (width-strlen(no_mem))/2, "%s", no_mem);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }

    newRoom->roomNumber = roomNumber;
    newRoom->scheduleCount = 0;
    newRoom->next = NULL;
    newRoom->prev = NULL;

    // Handle empty list case
    if (building->head == NULL) {
        building->head = building->last = newRoom;
    } 
    // Handle insertion at beginning
    else if (roomNumber < building->head->roomNumber) {
        newRoom->next = building->head;
        building->head->prev = newRoom;
        building->head = newRoom;
    }
    // Handle insertion at end
    else if (roomNumber > building->last->roomNumber) {
        building->last->next = newRoom;
        newRoom->prev = building->last;
        building->last = newRoom;
    }
    // Handle insertion in middle
    else {
        currentRoom = building->head;
        while (currentRoom->next != NULL && currentRoom->next->roomNumber < roomNumber) {
            currentRoom = currentRoom->next;
        }
        newRoom->next = currentRoom->next;
        if (currentRoom->next != NULL) {
            currentRoom->next->prev = newRoom;
        }
        newRoom->prev = currentRoom;
        currentRoom->next = newRoom;
    }

    _saveCurrentChanges(building);
    return 1; // Changed from -1 to 0 to indicate suprogCodeess
}
int _addBuiding(WINDOW *win,int height,int width) {
    int buildingNumber=_selectPromt(win,"Input Building Number: ");

    if(buildingNumber < 1){
        return -1;
    }

    // Check if building already exists
    struct Buildings *current = bHead;
    while(current != NULL) {
        if(current->buildingNumber == buildingNumber) {
            const char exists[] = "Building already exists";
            wattrset(win,A_REVERSE);
			mvwprintw(win,height/2,(width-strlen(exists))/2,"%s",exists);
            wattrset(win,A_NORMAL);
            return -1;
        }
        current = current->next;
    }
    
    int maxRooms=_selectPromt(win,"Input Max Room: ");
    if(maxRooms < 1) {
        return -1;
    }
    
    FILE* listOfBuildingsPtr = fopen("./buildings/current_changes/listOfBuildings.txt", "r");
    if(listOfBuildingsPtr == NULL) {
        const char no_file[]="Error opening listOfBuildings.txt";
        wattrset(win,A_REVERSE);
        mvwprintw(win,height/2,(width-strlen(no_file))/2,"%s",no_file);
        wattrset(win,A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }
    rewind(listOfBuildingsPtr);

    // Save current contents to changes file
    FILE *LOBuildingsChanges = fopen("./buildings/last_changes/listOfBuildingsChanges.txt", "wt");
    if(LOBuildingsChanges == NULL) {
        printf("Error opening changes file\n");
        fclose(listOfBuildingsPtr);
        exit(1);
    }

    char line[100];
    while(fgets(line, sizeof(line), listOfBuildingsPtr)) {
        fprintf(LOBuildingsChanges, "%s", line);
    }
    fclose(LOBuildingsChanges);
    
    struct Buildings* newBuilding = (struct Buildings *) malloc(sizeof(struct Buildings));
    if (!newBuilding) {
        const char no_mem[] = "Memory allocation failed";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height/2, (width-strlen(no_mem))/2, "%s", no_mem);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }

    newBuilding->buildingNumber = buildingNumber;
    newBuilding->maxRooms = maxRooms;
    newBuilding->head = NULL;
    newBuilding->last = NULL;
    newBuilding->next = NULL;
    newBuilding->prev = NULL;

    // First building
    if(bHead == NULL) {
        bHead = newBuilding;
        bLast = newBuilding;
    } 
    else if(buildingNumber > bLast->buildingNumber) {
        bLast->next = newBuilding;
        newBuilding->prev = bLast;
        bLast = newBuilding;
    }
    else if(buildingNumber < bHead->buildingNumber) {
        newBuilding->next = bHead;
        bHead->prev = newBuilding;
        bHead = newBuilding;
    }
    else {
        current = bHead;
        while(current->next != NULL) {
            if(buildingNumber < current->next->buildingNumber) {
                newBuilding->next = current->next;
                current->next->prev = newBuilding;
                newBuilding->prev = current;
                current->next = newBuilding;
                break;
            }
            current = current->next;
        }
    }

    _saveLastChanges(newBuilding);

    char bldFile[50];
    sprintf(bldFile, "./buildings/current_changes/bld%d.txt", buildingNumber);
    FILE* bldPtr = fopen(bldFile, "w");
    if(!bldPtr) {
        free(newBuilding);
        fclose(listOfBuildingsPtr);
        const char no_file[] = "Error creating building file";
        wattrset(win,A_REVERSE);
        mvwprintw(win, height/2, (width-strlen(no_file))/2, "%s", no_file);
        wattrset(win,A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }
    fprintf(bldPtr, "Building No: %d\n", buildingNumber);
    fprintf(bldPtr, "Max Rooms: %d\n", maxRooms);
    fclose(bldPtr);

    FILE *outFile = fopen("./buildings/current_changes/listOfBuildings.txt", "w");
    if(outFile == NULL) {
        printf("Error reopening file for writing\n");
        fclose(listOfBuildingsPtr);
        exit(1);
    }

    current = bHead;
    while(current != NULL) {
        fprintf(outFile, "bld%d.txt\n", current->buildingNumber);
        current = current->next;
    }

    _saveCurrentChanges(newBuilding);

    fclose(outFile);
    fclose(listOfBuildingsPtr);
    return -1;
}

//  show aba
int _deleteRoomSched(WINDOW *win,int height,int width,struct Rooms *room) {
    if (!room || room->scheduleCount == 0) {
        const char no_sched[] = "No schedules to delete";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height/2, (width-strlen(no_sched))/2, "%s", no_sched);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return 0;
    }

    // Get user input for which schedule to delete
    int rowToDelete = _selectPromt(win, "Enter schedule row to delete: ");
    if (rowToDelete <= 0 || rowToDelete > room->scheduleCount) {
        return 0; // Invalid input or cancelled
    }

    // Shift schedules down
    for(int i = rowToDelete-1; i < room->scheduleCount-1; i++) {
        room->schedules[i] = room->schedules[i+1];
    }
    room->scheduleCount--;

    // struct Buildings* building = bHead;
    // while (building != NULL) {
    //     if (building->head == room || building->last == room) {
    //         _saveLastChanges(building);
    //         _saveCurrentChanges(building);
    //         break;
    //     }
    //     building = building->next;
    // }

    // Show suprogCodeess message
    const char suprogCodeess[] = "Schedule deleted";
    wattrset(win, A_REVERSE);
    mvwprintw(win, height/2, (width-strlen(suprogCodeess))/2, "%s", suprogCodeess);
    wattrset(win, A_NORMAL);
    wrefresh(win);
    napms(1000);

    return 1;
}

void _deleteRoom(struct Buildings* currBuilding,int roomToDelete) {
    if (!currBuilding || !currBuilding->head) {
        return; // No building or no rooms to delete
    }

    _saveLastChanges(currBuilding);
    struct Rooms* currRoom = currBuilding->head;
    struct Rooms* toDelete;

    if(roomToDelete >= currBuilding->last->roomNumber) {    // Deletion at end
        toDelete = currBuilding->last;
        currBuilding->last = currBuilding->last->prev;
        currBuilding->last->next = NULL;
    } else if(roomToDelete <= currRoom->roomNumber) {       // Deletion at front
        toDelete = currRoom;
        currBuilding->head = currRoom->next;
        currBuilding->head->prev = NULL;
    } else {                                                // Deletion at any postiion
        while(currRoom!=NULL) {
            if(roomToDelete == currRoom->roomNumber) {
                // Delete room
                toDelete = currRoom;
                currRoom->prev->next = currRoom->next;
                currRoom->next->prev = currRoom->prev;
                break;  
            }
            currRoom = currRoom->next;
        }
    }

    _saveCurrentChanges(currBuilding);
    free(toDelete);
}

void _deleteBuilding(struct Buildings* currBuilding, int buildingToDelete) {
    _saveLastChanges(currBuilding);
    struct Buildings* toDelete = NULL;
    
    // Find the building to delete
    struct Buildings* current = bHead;
    while(current != NULL) {
        if(current->buildingNumber == buildingToDelete) {
            toDelete = current;
            break;
        }
        current = current->next;
    }
    
    if(toDelete == NULL) {
        return; // Building not found
    }
    
    // Update neighbors' pointers
    if(toDelete->prev != NULL) {
        toDelete->prev->next = toDelete->next;
    } else {
        // This was the head node
        bHead = toDelete->next;
    }
    
    if(toDelete->next != NULL) {
        toDelete->next->prev = toDelete->prev;
    } else {
        // This was the last node
        bLast = toDelete->prev;
    }
    
    // Free all rooms in the building first
    struct Rooms* currentRoom = toDelete->head;
    while(currentRoom != NULL) {
        struct Rooms* nextRoom = currentRoom->next;
        free(currentRoom);
        currentRoom = nextRoom;
    }
    
    // Now free the building
    free(toDelete);
    
    // Update the listOfBuildings.txt file (remove from active list)
    FILE* listFile = fopen("./buildings/current_changes/listOfBuildings.txt", "w");
    if(listFile == NULL) {
        return;
    }
    
    current = bHead;
    while(current != NULL) {
        fprintf(listFile, "bld%d.txt\n", current->buildingNumber);
        current = current->next;
    }

    _saveCurrentChanges(currBuilding);
    fclose(listFile);
}

// Transforms Uppercase words to lowercase
void Up2low(char word[10]) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = tolower(word[i]);
    }
}

int _addRoomSched(WINDOW *win, int height, int width, struct Rooms* room) {
    if (room->scheduleCount == MAX_SCHEDULES) {
        const char max_sched[] = "Maximum schedule reached for this room";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(max_sched)) / 2, "%s", max_sched);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        return -1;
    }

    // Get day
    char* day = _strSelectPromt(win, "Enter day: ");
    if (!day) return -1;

    int valid_day = 0;
    for (int i = 0; i < MAX_DAYS; i++) {
        if (strcasecmp(WEEK[i], day) == 0) {
            valid_day = 1;
            break;
        }
    }

    if (!valid_day) {
        const char msg[] = "Invalid day input";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        free(day);
        return -1;
    }

    // Get course code
    char* progCode = _strSelectPromt(win, "Enter Course Code (e.g., it1a, cs1a): ");
    if (!progCode) {
        free(day);
        return -1;
    }

    char programCode[15];
    strncpy(programCode, progCode, sizeof(programCode) - 1);
    programCode[sizeof(programCode) - 1] = '\0';
    free(progCode);
    Up2low(programCode);

    // Get time
    char* tm = _strSelectPromt(win, "Enter Time (e.g., 7am-9am): ");
    if (!tm) {
        free(day);
        return -1;
    }

    char time[15];
    strncpy(time, tm, sizeof(time) - 1);
    time[sizeof(time) - 1] = '\0';
    free(tm);
    Up2low(time);

    // Validate time format
    char firstHalf[10], secondHalf[10];
    if (sscanf(time, "%9[^-]-%9s", firstHalf, secondHalf) != 2) {
        const char msg[] = "Invalid time format (use 7am-9am or 7:00am-9:00am)";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        free(day);
        return -1;
    }

    // Validate time strings contain only digits, optional colon, and am/pm
    if (!_isValidTimeString(firstHalf) || !_isValidTimeString(secondHalf)) {
        const char msg[] = "Invalid time format (e.g., 7am, 7:30pm)";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        free(day);
        return -1;
    }

    int firstHalfInt, secondHalfInt;
    int firstHalfMin = 0, secondHalfMin = 0;

    // Parse first half
    if (strstr(firstHalf, ":")) {
        if (sscanf(firstHalf, "%d:%d", &firstHalfInt, &firstHalfMin) != 2) {
            const char msg[] = "Invalid time format (e.g., 7:30am)";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    } else {
        if (sscanf(firstHalf, "%d", &firstHalfInt) != 1) {
            const char msg[] = "Invalid time format (e.g., 7am)";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }

    // Parse second half
    if (strstr(secondHalf, ":")) {
        if (sscanf(secondHalf, "%d:%d", &secondHalfInt, &secondHalfMin) != 2) {
            const char msg[] = "Invalid time format (e.g., 7:30am)";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }
    else {
        if (sscanf(secondHalf, "%d", &secondHalfInt) != 1) {
            const char msg[] = "Invalid time format (e.g., 7am)";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }

    if(firstHalfInt > 12 || secondHalfInt > 12) {
        const char msg[] = "Invalid minutes (must be 1-12)";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        free(day);
        return -1;
    }

    // Get AM/PM indicators
    int isFAM = (firstHalf[strlen(firstHalf) - 2] == 'a') ? 1 : 0;
    int isSAM = (secondHalf[strlen(secondHalf) - 2] == 'a') ? 1 : 0;

    // Validate minutes
    if (firstHalfMin < 0 || firstHalfMin > 59 || secondHalfMin < 0 || secondHalfMin > 59) {
        const char msg[] = "Invalid minutes (must be 00-59)";
        wattrset(win, A_REVERSE);
        mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
        wattrset(win, A_NORMAL);
        wrefresh(win);
        napms(2000);
        free(day);
        return -1;
    }

    // Validate end time is after start time
    if (isFAM && isSAM) {
        if (secondHalfInt < firstHalfInt || 
            (secondHalfInt == firstHalfInt && secondHalfMin <= firstHalfMin)) {
            const char msg[] = "End time must be after start time";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }
    else if (!isFAM && !isSAM) {
        if (secondHalfInt < firstHalfInt || 
            (secondHalfInt == firstHalfInt && secondHalfMin <= firstHalfMin)) {
            const char msg[] = "End time must be after start time";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }
    else {
        // Mixed AM/PM (e.g., 11am-1pm)
        if (!(isFAM && !isSAM)) {
            const char msg[] = "Invalid time range (must be within AM or PM)";
            wattrset(win, A_REVERSE);
            mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
            wattrset(win, A_NORMAL);
            wrefresh(win);
            napms(2000);
            free(day);
            return -1;
        }
    }

    // Check for schedule conflicts
    for (int index = 0; index < room->scheduleCount; index++) {
        if (strcmp(room->schedules[index].day, day) == 0) {
            int schedFirstHalf, schedSecHalf, schedSecHalfMin;
            char schedFirstHalfStr[10], schedSecondHalfStr[10];
            char schedFirstHalfPeriod[3], schedSecondHalfPeriod[3];

            // Parse existing schedule time
            if (sscanf(room->schedules[index].time, "%9[^-]-%9s", schedFirstHalfStr, schedSecondHalfStr) != 2) {
                continue; // skip invalid entries
            }

            // Parse first half of existing schedule
            if (strstr(schedFirstHalfStr, ":")) {
                sscanf(schedFirstHalfStr, "%d:%d%2s", &schedFirstHalf, &schedSecHalfMin, schedFirstHalfPeriod);
            } else {
                sscanf(schedFirstHalfStr, "%d%2s", &schedFirstHalf, schedFirstHalfPeriod);
                schedSecHalfMin = 0;
            }

            // Parse second half of existing schedule
            if (strstr(schedSecondHalfStr, ":")) {
                sscanf(schedSecondHalfStr, "%d:%d%2s", &schedSecHalf, &schedSecHalfMin, schedSecondHalfPeriod);
            } else {
                sscanf(schedSecondHalfStr, "%d%2s", &schedSecHalf, schedSecondHalfPeriod);
                schedSecHalfMin = 0;
            }

            // Convert all times to minutes since midnight for easier comparison
            int newStart = convertToMinutes(firstHalfInt, firstHalfMin, isFAM);
            int newEnd = convertToMinutes(secondHalfInt, secondHalfMin, isSAM);
            int existingStart = convertToMinutes(schedFirstHalf, schedSecHalfMin, (schedFirstHalfPeriod[0] == 'a') ? 1 : 0);
            int existingEnd = convertToMinutes(schedSecHalf, schedSecHalfMin, (schedSecondHalfPeriod[0] == 'a') ? 1 : 0);
            
            // Check for overlap
            if ((newStart >= existingStart && newStart < existingEnd) ||
                (newEnd > existingStart && newEnd <= existingEnd) ||
                (newStart <= existingStart && newEnd >= existingEnd)) {
                const char msg[] = "Schedule conflict with existing booking";
                wattrset(win, A_REVERSE);
                mvwprintw(win, height / 2, (width - strlen(msg)) / 2, "%s", msg);
                wattrset(win, A_NORMAL);
                wrefresh(win);
                napms(2000);
                free(day);
                return -1;
            }
        }
    }

    // Store new schedule data
    struct Schedule *sched = &room->schedules[room->scheduleCount++];
    strcpy(sched->day, day);
    strcpy(sched->programCode, programCode);
    strcpy(sched->time, time);

    _sortSchedules(room);

}

int _isValidTimeString(const char *str) {
    int len = strlen(str);
    if (len < 2) return 0; // At least "1a" or "1p"
    
    // Check last two characters are am/pm
    char period = str[len-1];
    if (period != 'm' && period != 'M') return 0;
    char ampm = str[len-2];
    if (ampm != 'a' && ampm != 'A' && ampm != 'p' && ampm != 'P') return 0;
    
    // Check the rest are digits or colon
    for (int i = 0; i < len-2; i++) {
        if (!isdigit(str[i]) && str[i] != ':') return 0;
    }
    
    // Check there's at most one colon
    if (strchr(str, ':') != strrchr(str, ':')) return 0;
    
    return 1;
}

int convertToMinutes(int hour, int minute, int isAM) {
    if (hour == 12) {
        hour = isAM ? 0 : 12; // 12am is 0, 12pm is 12
    } else {
        hour = isAM ? hour : hour + 12;
    }
    return hour * 60 + minute;
}


// Sort Schedules using the bubble sort Algo
void _sortSchedules(struct Rooms* room) {
    // Sort schedules per day using bubble sort
    for (int i = 0; i < room->scheduleCount - 1; i++) {
        for (int k = 0; k < room->scheduleCount - i - 1; k++) {
            int dayIndex = 0, dayIndex2 = 0;
            
            // Find the index of the first day
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[k].day, WEEK[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }
            
            // Find the index of the second day
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[k+1].day, WEEK[index]) == 0) {
                    dayIndex2 = index;
                    break;
                }
            }
            
            // If days are out of order, perform swap
            if (dayIndex > dayIndex2) {
                struct Schedule temp = room->schedules[k];
                room->schedules[k] = room->schedules[k+1];
                room->schedules[k+1] = temp;
            }
        }
    }

    // Then sort by time within each day
    for (int i = 0; i < room->scheduleCount - 1; i++) {
        for (int k = 0; k < room->scheduleCount - i - 1; k++) {
            int firstHalf, secondHalf;
            char fTime[11], sTime[11];


            sscanf(room->schedules[k].time, "%10[^-]-", fTime);
            sscanf(room->schedules[k+1].time, "%10[^-]-", sTime);
            sscanf(room->schedules[k].time, "%d", &firstHalf);
            sscanf(room->schedules[k+1].time, "%d", &secondHalf);\
            
            int isFirstPM = (fTime[strlen(fTime) - 2] == 'p') ? 1 : 0;
            int isSecondPM = (sTime[strlen(sTime) - 2] == 'p') ? 1 : 0;

            //convert to 24 format if pm
            if(isFirstPM && firstHalf != 12) 
                firstHalf += 12;
            
            if(isSecondPM && secondHalf != 12)
                secondHalf += 12;
            
            // Only compare times if the days are the same
            if (strcasecmp(room->schedules[k].day, room->schedules[k+1].day) == 0) {
                // Compare start times
                if (firstHalf > secondHalf) {
                    struct Schedule temp = room->schedules[k];
                    room->schedules[k] = room->schedules[k+1];
                    room->schedules[k+1] = temp;
                }
            }
        }
    }
}

void freeAllLists() {
    struct Buildings *current = bHead;
    while (current != NULL) {
        struct Buildings *next = current->next;
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
    bHead = bLast = NULL;
}