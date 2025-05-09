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

char* WEEK[MAX_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int Select_prompt(WINDOW *win,char input_text[]);
void Print_bldng(WINDOW *win,int height,int width,char status[]);
void Print_rooms(WINDOW *win,int height,int width,struct Buildings *building,char status[]);
void Load_sched(struct Rooms *room, int dayIndex, const char *courseCode, const char *time);
struct Buildings* Load_bldngs(int buildingNumber, int maxRms);
struct Rooms* Load_room(int roomNumber, struct Buildings *currentBuilding);
struct Rooms* Select_room(WINDOW *win,int height,int width,int roomNumber, struct Buildings* currentBuilding);
struct Buildings* Select_bldng(WINDOW *win,int height,int width,int bNum);
void Print_sched(WINDOW *win,int height,int width,struct Buildings *building, struct Rooms* room);
void Save_cur_changes(struct Buildings *current);
void Save_last_changes(struct Buildings *current);
void Edit_room_schedule(struct Rooms *room);
void addRoom(struct Buildings *building);
int Add_building(WINDOW *win,int height,int width);
void Del_roomsched(struct Rooms *room);
void Del_room(struct Buildings* currBuilding);
void Del_bldng(struct Buildings* currBuilding, int buildingToDelete);
void Up2low(char word[10]);
void Add_roomsched(struct Rooms* room);
void Sort_sched(struct Rooms* room);
void Edit_bldng(struct Buildings *building);
void Revert_changes(struct Buildings *current);
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
            struct Buildings *building = Load_bldngs(bNumber, maxRooms);
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
                    room = Load_room(currentRoom, building);
                    continue;
                }
    
                if (strlen(line) <= 1)
                    continue; // If empty line skip.
    
                int dayIndex;
                char courseCode[21], time[21];
    
                if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
                    if (room)
                        Load_sched(room, dayIndex, courseCode, time);
            }
            fclose(CBPtr);
        }
        struct Buildings *currentBuilding = bHead;
        while (currentBuilding != NULL) {
            struct Rooms *currentRoom = currentBuilding->head;
            while (currentRoom != NULL) {
                Sort_sched(currentRoom);
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
        "[2] Edit Building  ",
        "[3] Delete Building",
        "[4] View Building  ",
        "[5] Revert Changes ",
    };
    int bnt_row_size=sizeof(bldng_nav_text)/sizeof(bldng_nav_text[0]);


    const char *room_nav_text[]={
        "[1] Add Room   ",
        "[2] Delete Room",
        "[3] Edit Room  ",
        "[4] View Room  ",
    };
    int rnt_row_size=sizeof(room_nav_text)/sizeof(room_nav_text[0]);

    const char *sched_nav_text[]={
        "[1] Add Schedule      ",
        "[2] Delete Schedule   ",
        "[3] Edit Schedule     ",
        "[4] Print Last Changes"
    };
    int snt_row_size=sizeof(sched_nav_text)/sizeof(sched_nav_text[0]);

    char ext[]="[X] Exit";

    fclose(LOBPtr);

    int flag = 0;
// Sort each schedules per buildings and rooms
    while(flag!=-1) {
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

        int bNum;
        struct Buildings* selectedBuilding;
        struct Rooms* selectedRoom;
        int buildingChoice;
        int roomOfChoice;

        int ch_bldng=0;

        ch_bldng=wgetch(win);
        if(flag==0){
            if(!bytes_read && ch_bldng!='1'){
                const char no_bldng_avail[]="Buildings unavailable, please add buildings first.";
                wattrset(win,A_REVERSE);
				mvwprintw(win,height/2,(width-strlen(no_bldng_avail))/2,"%s",no_bldng_avail);
                wattrset(win,A_NORMAL);
				wrefresh(win);
				napms(2000);
                break;
            }

            if(ch_bldng=='1'){
                Print_bldng(win,height,window_width,"Admin/Add Building");
                int addBldng=Add_building(win,height,window_width);
                if(addBldng!=-1){
                    flag=0;
                }
            }
            else if(ch_bldng=='2'){

            }
            else if(ch_bldng=='3'){

            }
            else if(ch_bldng=='4'){
                Print_bldng(win,height,window_width,"Admin/View Building");
                buildingChoice=Select_prompt(win,"Input Building: ");
                if(buildingChoice!=-1){
                    selectedBuilding=Select_bldng(win,height,window_width,buildingChoice);
                    if(!selectedBuilding){
                        continue;
                    }
                    flag=1;
                }
            }
            else if(ch_bldng=='5'){

            }

            else if(toupper(ch_bldng)==('X')){
                flag=-1;
                freeAllLists();
                delwin(win);
                break;
            }
            wrefresh(win);
            check_winsize(win,height,window_width);
        }
        while(flag==1){
            wclear(win);
            status_bar(win,"Admin/Buildigs/Rooms");
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

            int ch_room;
            ch_room=wgetch(win);
            if(ch_room=='1'){
                continue;
            }
            else if(ch_room=='2'){

            }
            else if(ch_room=='3'){

            }
            else if(ch_room=='4'){
                Print_rooms(win,height,window_width,selectedBuilding,"Admin/View Building/");
                int roomChoice=Select_prompt(win,"Input Room: ");
                if(roomChoice!=-1){
                    selectedRoom=Select_room(win,height,window_width,roomChoice,selectedBuilding);
                    if(!selectedRoom){
                        flag=1;
                        continue;
                    }
                    flag=2;
                }
            }
            else if(toupper(ch_room)==('X')){
                flag=0;
                selectedBuilding=NULL;
                break;
            }
            wrefresh(win);
            check_winsize(win,height,window_width);

            while(flag==2){
                wclear(win);
                status_bar(win,"Admin/Buildigs/Rooms/Schedules");
                wborder(win,'|','|','-','-','+','+','+','+');
    
                for(int i=0;i<adt_row_size;i++){
                    int len=strlen(admin_text[i]);
                    int tab=(window_width-len)/2;
                    mvwprintw(win,i+3,tab,"%s",admin_text[i]);
                }
    
                for(int i=0;i<snt_row_size;i++){
                    int len=strlen(sched_nav_text[i]);
                    int tab=(window_width-len)/2;
                    mvwprintw(win,(height/2)-rnt_row_size+i*2,tab,"%s",sched_nav_text[i]);
                }
    
                mvwprintw(win,height-4,(window_width-strlen(ext))/2,"%s",ext);
    
                wrefresh(win);
    
                int ch_sched;
                ch_sched=wgetch(win);
                if(ch_sched=='1'){
    
                    continue;
                }
                else if(ch_sched=='2'){
    
                }
                else if(ch_sched=='3'){
    
                }
                else if(ch_sched=='4'){
                    Print_sched(win,height,window_width,selectedBuilding,selectedRoom);
                }
                else if(toupper(ch_sched)==('X')){
                    flag=1;
                    selectedRoom=NULL;
                    break;
                }
                wrefresh(win);
                check_winsize(win,height,window_width);
            }
        }
        wrefresh(win);
		check_winsize(win,height,window_width);
    }
}

int Select_prompt(WINDOW *win,char input_text[]){
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

/**
 * @date_added: 4/16
 * @date_edited: 05/09/2025
 * @return_type: void
 * @description: Prints the list of Buildingss
 */

void Print_bldng(WINDOW *win,int height,int width,char status[]) {
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
 * @parameter: Accepts an Building type Structure
 * @description: Prints the list of Rooms
 */
void Print_rooms(WINDOW *win,int height,int width,struct Buildings *building,char status[]) {
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
 * @parameter: Accepts a Room type Structure, Int, Char 
 * @description: Adds data to an Array of Schedule Structure of the cuurent Room object passed as an argument
 */
void Load_sched(struct Rooms *room, int dayIndex, const char *courseCode, const char *time) {
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
 * @parameter: Accepts an Int
 * @description: Creates a linked list of Building
 */
struct Buildings* Load_bldngs(int buildingNumber, int maxRms) {
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
 * @parameter: Accepts an INt, Buildings Structure
 * @description: Creates a linked list of Rooms inside a structure of Building
 */
struct Rooms* Load_room(int roomNumber, struct Buildings *currentBuilding) {

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
 * @parameter: Accepts an INt, Buildings Structure
 * @description: returns a Rooms Structure selected by the user for purposes such as Printing, Updating, Deleting of Room
 */
struct Rooms* Select_room(WINDOW *win,int height,int width,int roomNumber,struct Buildings* currentBuilding) {
    struct Rooms* current = currentBuilding->head;
    while (current != NULL) {
        if (current->roomNumber == roomNumber) { // if val of current->roomNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }

        if (roomNumber > currentBuilding->last->roomNumber) {
            return currentBuilding->last;
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
 * @parameter: Accepts an INt
 * @description: returns a Buildings Structure selected by the user to be able to access Rooms Objects of that Buildings Structure
 */
struct Buildings* Select_bldng(WINDOW *win,int height,int width,int bNum) {
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
 * @date_added: 4/15
 * @return_type: void
 * @parameter: Accepts Buildings Structure, And Rooms Structure
 * @description: The Structure Arguments are the structures acquired from the selectBuilding and selectRoom functions, the printSelectedRoom
 * function prints the data contents of an specific room.
 */
void Print_sched(WINDOW *win,int height,int width,struct Buildings *building, struct Rooms* room) {
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
        int cc_len=strlen(room->schedules[i].programCode);
        int time_len=strlen(room->schedules[i].time);
        int total_len=day_len+cc_len+time_len;
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

// Save updated version
void Save_cur_changes(struct Buildings *current) {
    // savePTR; current saved datas will be stored;
    FILE* savePTR;
    char strBuildingNumber[5];
    sprintf(strBuildingNumber, "%d", current->buildingNumber);
    char dirCurrent[125] = "./buildings/current_changes/bld";
    strcat(dirCurrent, strBuildingNumber);
    strcat(dirCurrent, ".txt");

    savePTR = fopen(dirCurrent, "wt");

    fprintf(savePTR, "Building No: %d\n", current->buildingNumber);
    fprintf(savePTR, "Max Rooms: %d\n", current->maxRooms);
    struct Rooms *room = current->head;
    while(room!=NULL) {
        fprintf(savePTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, WEEK[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }
            fprintf(savePTR, "%d, %s, %s\n", dayIndex, room->schedules->programCode, room->schedules->time);
        }
        fprintf(savePTR, "\n");
        room = room->next;
    }
    fclose(savePTR);
}

void Save_last_changes(struct Buildings *current) {
    // changesPTR; where all the changes history will be written
    FILE* changesPTR;
    char strBuildingNumber[5];
    int buildingNumber = current->buildingNumber;
    int maxRooms = current->maxRooms;
    sprintf(strBuildingNumber, "%d", buildingNumber);
    char dirChanges[125] = "./buildings/last_changes/last_changes_bld";
    strcat(dirChanges, strBuildingNumber);
    strcat(dirChanges, ".txt");

    changesPTR = fopen(dirChanges, "wt");
    fprintf(changesPTR, "Building No: %d\n", buildingNumber);
    fprintf(changesPTR, "Max Rooms: %d\n", maxRooms);

    struct Rooms *room = current->head;
    while(room!=NULL) {
        fprintf(changesPTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, WEEK[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }

            fprintf(changesPTR, "%d, %s, %s\n", dayIndex, room->schedules->programCode, room->schedules->time);
        }
        fprintf(changesPTR, "\n");
        room = room->next;
    }
    fclose(changesPTR);
}

void Edit_room_schedule(struct Rooms *room) {
    int rowToEdit;
    for(int row=0; row < room->scheduleCount; row++) {
        printf("%d. %s, %s %s\n", row, room->schedules[row].day, room->schedules[row].programCode, room->schedules[row].time);
    }
    printf("Enter Row to edit");
    scanf("%d", &rowToEdit);

    struct Schedule *current = &room->schedules[rowToEdit];
    printf("%s, %s, %s\n", current->day, current->programCode, current->time);
    printf("What do you want to delete: [d]day, [c]oursecode, [t]ime, [a]ll");
    char option;
    char day[20], coursecode[10], time[20];
    scanf(" %c", &option);
    switch(option) {
        case 'd':
            printf("Enter Day: ");
            scanf("%s", day);
            strcpy(current->day, day);
            break;
        case 'c':
            printf("Enter CourseCode");
            scanf("%s", coursecode);
            strcpy(current->programCode, coursecode);
            break;
        case 't':
            printf("Enter Time");
            strcpy(current->time, time);
            scanf("%s", time);
            break;
        case 'a':
            printf("Enter Day: ");
            scanf("%s", day);
            strcpy(current->day, day);
            printf("Enter CourseCode");
            scanf("%s", coursecode);
            strcpy(current->programCode, coursecode);
            printf("Enter Time");
            scanf("%s", time);
            scanf("%s", time);
        default: 
            printf("Invalid\n");
    }
}

void Revert_changes(struct Buildings *current) {
    FILE* revertPtr;

    Save_last_changes(current); // Save muna current configurations

    char strBuildingNumber[5];
    int buildingNumber = current->buildingNumber;
    sprintf(strBuildingNumber, "%d", buildingNumber);
    char dirChanges[125] = "./buildings/last_changes/last_changes_bld";
    strcat(dirChanges, strBuildingNumber);
    strcat(dirChanges, ".txt");

    revertPtr = fopen(dirChanges, "rt");

    char line[100];
    fgets(line, sizeof(line), revertPtr);
    sscanf(line, "Building No: %d", &current->buildingNumber);
    fgets(line, sizeof(line), revertPtr);
    sscanf(line, "Max Rooms: %d", &current->maxRooms);

    struct Rooms *currentRm = current->head;

    while (fgets(line, sizeof(line), revertPtr)) {
        // If Room: is present on the string
        if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
            int currentRoom = 0;
            sscanf(line, "Room: %d", &currentRoom);
            currentRm->roomNumber = currentRoom;
            continue;
        }

        if (strlen(line) <= 1)
            continue; // If empty line skip.

        int dayIndex;
        char courseCode[21], time[21];

        if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
        if (currentRm)
            Load_sched(currentRm, dayIndex, courseCode, time);
    }
    Save_cur_changes(current);
    fclose(revertPtr);
}

void addRoom(struct Buildings *building) {
    // Check if room number is less than max
    // struct Rooms* currentRoom = building->last;
    // printf("Last Room no: %d\n\n\n", currentRoom->roomNumber);
    int roomCount=0;
    struct Rooms* currentRoom = building->head;
    while(currentRoom != NULL) {
        roomCount++;
        currentRoom = currentRoom->next;
    }
    if(roomCount >= building->maxRooms) {
        // Then nde na pedeng magadd ng room
        return;
    }

    // Proceed if pede pa magadd ng room
    printf("Enter Room Number: (e.g. 101, 201)");
    int roomNumber;
    scanf("%d", &roomNumber);
    
    // To check if roomNumber already exist
    while(currentRoom != NULL) {
        if(roomNumber == currentRoom->roomNumber) {
            // Then room already exist
            // ask user if usto pa magcontinue if yes recursion
            // call the same function again
            addRoom(building);
        }
        currentRoom = currentRoom->next;
    }
    
    struct Rooms *newRoom = (struct Rooms *) malloc(sizeof(struct Rooms));

    newRoom->roomNumber = roomNumber;
    newRoom->scheduleCount = 0;
    newRoom->next = NULL;
    newRoom->prev = NULL;

    // Insert the the end if roomNumber is greater than last of room
    if(roomNumber > building->last->roomNumber) {
        building->last->next = newRoom; 
        newRoom->prev = building->last;
        return;
    }

    // Insert front if roomNumber less than head of room
    if(roomNumber < building->head->roomNumber) {
        newRoom->next = building->head; // next ng newroom naka point sa kasunod na list ng currentRoom
        building->head->prev = newRoom; // prev ng kasunod ng currentRoom nakapoint sa newRoom
        building->head = newRoom;
        return;
    }

    currentRoom = building->head;
    while(currentRoom != NULL) {
        if(roomNumber < currentRoom->next->roomNumber) {
            newRoom->next = currentRoom->next; // next ng newroom naka point sa kasunod na list ng currentRoom
            currentRoom->next->prev = newRoom; // prev ng kasunod ng currentRoom nakapoint sa newRoom

            newRoom->prev = currentRoom;
            currentRoom->next = newRoom;
            return;
        }
        currentRoom = currentRoom->next;
    }
}
// edit specific building info e.g. maxrooms, buildingnumber, etc
void Edit_bldng(struct Buildings *building) {
    int currentBNumber = building->buildingNumber; // for error handling puruposes
    char option;
    printf("What do you want to edit: [b]uildingNumber, [m]axRooms, [a]ll");
    scanf(" %c", &option);

    int newBuildingNumber, newMaxRooms;
    struct Buildings *current = bHead; // will be used to traverse the linkedlist of Buildings structure

    switch (option) {
        case 'b':   
            printf("Enter new building number: ");
            scanf("%d", &newBuildingNumber);
            if(newBuildingNumber<1) return; // error negative value
            if(newBuildingNumber == currentBNumber) {
                printf("newBuildingNubmer is equal to the currentBNumber..do you want to [e]exit or [r]try");
                char exitOrRetry;
                scanf(" %c", &exitOrRetry);

                if(exitOrRetry == 'e')
                    return;
                else
                    Edit_bldng(building);
            }
            //TODO: check if existing na building number
            while(current!=NULL) {
                if(newBuildingNumber == current->buildingNumber) {
                    printf("Building Nubmer already existed\n");
                    return;
                }
                current = current->next;
            }
            building->buildingNumber = newBuildingNumber;
            break;
        case 'm':
            printf("Enter new maxRooms count: ");
            scanf("%d", &newMaxRooms);
            if(newMaxRooms<1) return; // error
            building->maxRooms = newMaxRooms;
            break;
        case 'a':
            printf("Enter new building number: ");
            scanf("%d", &newBuildingNumber);
            if(newBuildingNumber<1) return; // error negative value
            if(newBuildingNumber == currentBNumber) {
                printf("newBuildingNubmer is equal to the currentBNumber..do you want to [e]exit or [r]try");
                char exitOrRetry;
                scanf(" %c", &exitOrRetry);

                if(exitOrRetry == 'e')
                    return;
                else
                    Edit_bldng(building);
            }
            //TODO: check if existing na building number
            while(current!=NULL) {
                if(newBuildingNumber == current->buildingNumber) {
                    printf("Building Nubmer already existed\n");
                    return;
                }
                current = current->next;
            }
            building->buildingNumber = newBuildingNumber;
            printf("Enter new maxRooms count: ");
            scanf("%d", &newMaxRooms);
            if(newMaxRooms<1) return; // error
            building->maxRooms = newMaxRooms;
            break;
        default:
            printf("Invalid");
    }
}

int Add_building(WINDOW *win,int height,int width) {
    int buildingNumber=Select_prompt(win,"Input Building Number: ");

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
    
    int maxRooms=Select_prompt(win,"Input Max Room: ");
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
    
    fclose(outFile);
    fclose(listOfBuildingsPtr);
}

//  show aba
void Del_roomsched(struct Rooms *room) {
    int currentRoomNumber = room->roomNumber;
    printf("Room Number: %d\n", currentRoomNumber);
    printf("Enter Row to delete: ");

    int rowToDelete;
    scanf("%d", &rowToDelete);

    for(int i = rowToDelete; i < room->scheduleCount -1; i++) {
       room->schedules[i] = room->schedules[i + 1];
    }

    room->scheduleCount--;

    for (int i = 0; i < room->scheduleCount; i++) {
        printf("%d.  %s, %s at %s\n",
               i,
               room->schedules[i].day,
               room->schedules[i].programCode,
               room->schedules[i].time);
    }
}

void Del_room(struct Buildings* currBuilding) {
    struct Rooms* currRoom = currBuilding->head;
    struct Rooms* toDelete;

    int roomToDelete;
    printf("Enter room to delete: ");
    scanf("%d", &roomToDelete);

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
    free(toDelete);
}

void Del_bldng(struct Buildings* currBuilding, int buildingToDelete) {
    struct Buildings* currentBuilding = bHead;
    struct Buildings* toDelete;
    
    if(buildingToDelete >= bLast->buildingNumber) {    // Deletion at end
        toDelete = bLast;
        bLast = bLast->prev;
        bLast->next = NULL;
    } else if(buildingToDelete <= currentBuilding->buildingNumber) {       // Deletion at front
        toDelete = bHead;
        bHead = bHead->next;
        bHead->prev = NULL;
    } else {                                                // Deletion at any postiion
        while(currentBuilding!=NULL) {
            if(buildingToDelete == currentBuilding->buildingNumber) {
                // Delete room
                toDelete = currentBuilding;
                currentBuilding->prev->next = currentBuilding->next;
                currentBuilding->next->prev = currentBuilding->prev;
                break;  
            }
            currentBuilding = currentBuilding->next;
        }
    }
    free(toDelete);
}

// Transforms Uppercase words to lowercase
void Up2low(char word[10]) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = tolower(word[i]);
    }
}

void Add_roomsched(struct Rooms* room) {
    if (room->scheduleCount == MAX_SCHEDULES) {
        printf("Maximum schedules reached\n");
        return;
    }

    int currentRoomNumber = room->roomNumber;
    printf("\nRoom Number: %d\n", currentRoomNumber);
    
    char day[15], coursecode[15], time[15];
    printf("Enter day: ");
    scanf("%s", day);
    printf("Enter coursecode: (it1a, cs1a): ");
    scanf("%s", coursecode);
    printf("Enter time: (e.g. 7am-9am): ");
    scanf("%s", time);

    // just to make sure everytbing's in lowercase
    Up2low(day);
    Up2low(coursecode);
    Up2low(time);

    // printf("%s, %s, %s", day, coursecode, time);
    // printf("%s", day);
    // printf("   %s\n", DAYS[0]);

    // Checks if user inputted day exists on the DAYS array.
    for (int i = 0; i <= MAX_DAYS; i++) {
        if(i >= MAX_DAYS) {
            printf("Invalid day input, please try again.\n");
            Add_roomsched(room);
        }
        if(strcmp(WEEK[i], day) == 0)
            break;
    }

    // Error handling for the time input string
    char firstHalf[10]; // First half of the hh:mmxx - hh:mmxx
    char secondHalf[10]; // Second half of the hh:mmxx - hh:mmxx
    sscanf(time, "%9[^-]-%s", firstHalf, secondHalf);
    // For debugging purposes lang to
    printf("First half: %s\n", firstHalf);
    printf("second half: %s\n", secondHalf);
    
    int FHI; // First half of hour but int; 11pm the 11 will be stored
    int SHI; // likewise...katamad magcomment
    int FHMI = 0; // First half of time but minutes; HH:MMxx; MM
    int SHMI = 0;
    
    // checks if both first and second half len of string is equal 7 (hh:mmxx) then capture that minute value as well
    if (strlen(firstHalf) > 4 && strlen(secondHalf) > 3) {
        sscanf(firstHalf, "%d:%d", &FHI, &FHMI);
        sscanf(secondHalf, "%d:%d", &SHI, &SHMI);
    } else if(strlen(firstHalf) > 4 && strlen(secondHalf) <= 4) {
        sscanf(firstHalf, "%d:%d", &FHI, &FHMI);
        sscanf(secondHalf, "%d", &SHI);
    } else if(strlen(firstHalf) <= 4 && strlen(secondHalf) > 4) {
        sscanf(firstHalf, "%d", &FHI);
        sscanf(secondHalf, "%d:%d", &SHI, &SHMI);
    } else {
        sscanf(firstHalf, "%d", &FHI);
        sscanf(secondHalf, "%d", &SHI);
    }
    //TODO: Create an error handling to handle overlapping schedules on a room
    for (int index=0; index < room->scheduleCount; index++) {
        if(strcmp(room->schedules[index].day, day) == 0)  {// if same day as inputted value
            int schedFirstHalf, schedSecHalf,schedSecHalfMin;
            sscanf(room->schedules[index].time, "%d", &schedFirstHalf);//retrieves the first half of hh:mmxx-hh:mmxx
            char trsh[51]; // ignore
            sscanf(room->schedules[index].time, "%50[^-]-%d:%d", trsh, &schedSecHalf, &schedSecHalfMin);
            if((FHI == schedFirstHalf) || (FHI == schedSecHalf && FHMI < schedSecHalfMin)) {//e.g 7 and 7:30, then error
                printf("Error overlapping values\n");
                return;
            }
        }
    }

    // TODO: create an error handling for random input string for each fields e.g. aslfjk-lsakjflam

    // Below this shit are buggy ass error handling, will improve later
    int isFAM = (firstHalf[strlen(firstHalf) - 2] == 'a') ? 1 : 0; // 1 if AM 0 if PM
    int isSAM = (secondHalf[strlen(secondHalf) - 2] == 'a') ? 1 : 0; // 1 if AM 0 if PM

    if(firstHalf[strlen(firstHalf) - 2] == secondHalf[strlen(secondHalf) - 2]) { 
        if(SHI < FHI) { // if it's AM or PM and Second half is higher than first half then error
            printf("3Invalid time values, Please try again\n");
            Add_roomsched(room);
        }
    }
    // HH:mm; if HH if over 12 then error(this aint military time bruv), if HH is less than 6 then error there's no way someone's class starts at 5
    // HH:mm; if HH if over 10(cause u cant have classes at 11 en't u?) or less than 1 then error(this aint military time bruv)
    if(isFAM && isSAM) {
        if(FHI > 12 || FHI < 6 || SHI > 12 || SHI < 6) { 
            printf("1Invalid time values, please try again\n");
            Add_roomsched(room);
        }
        if(SHI == 12) {
            printf("1Invalid time values, please try again\n");
            Add_roomsched(room);
        }
    } else if(!isFAM && !isSAM) {
        if (FHI > 10 || FHI < 1 || SHI > 10 || SHI < 1) {
            printf("11Invalid time values, please try again\n");
            Add_roomsched(room);
        }
        if(SHI == 12) {
            printf("1Invalid time values, please try again\n");
            Add_roomsched(room);
        }
    }

    if(FHMI || SHMI) // if  hindi 0 ang value ng minutes
        if((FHMI > 59 || FHMI < 0) || (SHMI > 59 || SHMI < 0)) {
            printf("2Invalid time value, please try again.\n");
            Add_roomsched(room);
        }
    
    // Store add new Datas to the array of Schedule structures of the current Room structure passed
    struct Schedule *sched = &room->schedules[room->scheduleCount++];
    strcpy(sched->day, day);
    strcpy(sched->programCode, coursecode);
    strcpy(sched->time, time);

    for (int i = 0; i < room->scheduleCount; i++) 
        printf("%d.  %s, %s at %s\n",
               i,
               room->schedules[i].day,
               room->schedules[i].programCode,
               room->schedules[i].time);
}


// Sort Schedules using the bubble sort Algo
void Sort_sched(struct Rooms* room) {
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
    struct Buildings* currentBuilding = bHead;
    while (currentBuilding!=NULL) {
        struct Rooms* currentRooms = currentBuilding->head;
        while (currentRooms!=NULL) {
            struct Rooms* temp = currentRooms;
            currentRooms = currentRooms->next;
            free(temp);
        }
        struct Buildings* temp = currentBuilding;
        currentBuilding = currentBuilding->next;
        free(temp);
    }
}