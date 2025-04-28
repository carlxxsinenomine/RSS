#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>

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
};


struct Buildings {
    int buildingNumber;
    int maxRooms;
    struct Rooms* head;
    struct Rooms* last;
    struct Buildings *prev;
    struct Buildings *next;
} *bHead=NULL, *bLast=NULL;

char* DAYS[MAX_DAYS] = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
/**
 * @date_added: 4/16
 * @return_type: void
 * @description: Prints the list of Buildingss
 */
void printBuildings() {
    if (bHead == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    struct Buildings *current = bHead;
    printf("Room List:\n");
    while (current != NULL) {
        printf("Building %d:\n", current->buildingNumber);
        current = current->next;
    }
}
/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: Accepts an Building type Structure
 * @description: Prints the list of Rooms
 */
void printRooms(struct Buildings *building) {
    if (building->head == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    struct Rooms *current = building->head;
    printf("Room List:\n");
    while (current != NULL) {
        printf("Room %d:\n", current->roomNumber);
        // for (int i = 0; i < current->scheduleCount; i++) {
        //     printf("  %s, %s at %s\n", 
        //            current->schedules[i].day,
        //            current->schedules[i].courseCode,
        //            current->schedules[i].time);
        // }
        current = current->next;
    }
}
/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: Accepts a Room type Structure, Int, Char 
 * @description: Adds data to an Array of Schedule Structure of the cuurent Room object passed as an argument
 */
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
/**
 * @date_added: 4/16
 * @return_type: Buildings Structure
 * @parameter: Accepts an Int
 * @description: Creates a linked list of Building
 */
struct Buildings* createBuilding(int buildingNumber, int maxRms) {
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
struct Rooms* createRoom(int roomNumber, struct Buildings *currentBuilding) {

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
struct Rooms* selectRoom(int roomNumber, struct Rooms* head) {
    struct Rooms* current = head;
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
/**
 * @date_added: 4/16
 * @return_type: Buildings Structure
 * @parameter: Accepts an INt
 * @description: returns a Buildings Structure selected by the user to be able to access Rooms Objects of that Buildings Structure
 */
struct Buildings* selectBuilding(int bNum) {
    struct Buildings* current = bHead;
    while (current != NULL) {
        if (current->buildingNumber == bNum) { // if val of current->roomNumber is equal to current edi same building
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

/**
 * @date_added: 4/15
 * @return_type: void
 * @parameter: Accepts Buildings Structure, And Rooms Structure
 * @description: The Structure Arguments are the structures acquired from the selectBuilding and selectRoom functions, the printSelectedRoom
 * function prints the data contents of an specific room.
 */
void printSelectedRoom(struct Rooms* room) {
    int currentRoomNumber = room->roomNumber;
    printf("Room Number: %d\n", currentRoomNumber);
    // printf("Room Sched Count: %d", room->scheduleCount);
    if (room == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    for (int i = 0; i < room->scheduleCount; i++) {
        printf("%d.  %s, %s at %s\n",
               i,
               room->schedules[i].day,
               room->schedules[i].courseCode,
               room->schedules[i].time);
    }
}

// Save updated version
void _saveCurrentChanges(struct Buildings *current) {
    // savePTR; current saved datas will be stored;
    FILE* savePTR;
    char strBuildingNumber[5];
    sprintf(strBuildingNumber, "%d", current->buildingNumber);
    char dirCurrent[125] = "./buildings/current_changes/bld";
    strcat(dirCurrent, strBuildingNumber);
    strcat(dirCurrent, ".txt");

    printf("\n\n%s\n\n", dirCurrent);
    savePTR = fopen(dirCurrent, "wt");

    fprintf(savePTR, "Building No: %d\n", current->buildingNumber);
    fprintf(savePTR, "Max Rooms: %d\n\n", current->maxRooms);

    struct Rooms *room = current->head;
    while(room!=NULL) {
        fprintf(savePTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, DAYS[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }

            fprintf(savePTR, "%d, %s, %s\n", dayIndex, room->schedules->courseCode, room->schedules->time);
        }
        fprintf(savePTR, "\n");
        room = room->next;
    }
    fclose(savePTR);
}

void _saveLastChanges(struct Buildings *current) {
    // changesPTR; where all the changes history will be written
    FILE* changesPTR;
    char strBuildingNumber[5];
    int buildingNumber = current->buildingNumber;
    int maxRooms = current->maxRooms;
    sprintf(strBuildingNumber, "%d", buildingNumber);
    char dirChanges[125] = "./buildings/last_changes/last_changes_bld";
    strcat(dirChanges, strBuildingNumber);
    strcat(dirChanges, ".txt");
    printf("\n\n%s\n\n", dirChanges);

    changesPTR = fopen(dirChanges, "wt");
    fprintf(changesPTR, "Building No: %d\n", buildingNumber);
    fprintf(changesPTR, "Max Rooms: %d\n\n", maxRooms);

    struct Rooms *room = current->head;
    while(room!=NULL) {
        fprintf(changesPTR, "Room: %d\n", room->roomNumber);
        for(int i=0; i < room->scheduleCount; i++) {
            int dayIndex=0;
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[i].day, DAYS[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }

            fprintf(changesPTR, "%d, %s, %s\n", dayIndex, room->schedules->courseCode, room->schedules->time);
        }
        fprintf(changesPTR, "\n");
        room = room->next;
    }
    fclose(changesPTR);
}

void editRoomSchedule(struct Rooms *room) {
    int rowToEdit;
    for(int row=0; row < room->scheduleCount; row++) {
        printf("%d. %s, %s %s\n", row, room->schedules[row].day, room->schedules[row].courseCode, room->schedules[row].time);
    }
    printf("Enter Row to edit");
    scanf("%d", &rowToEdit);

    struct Schedule *current = &room->schedules[rowToEdit];
    printf("%s, %s, %s\n", current->day, current->courseCode, current->time);
    printf("What do you want to delete: [d]day, [c]oursecode, [t]ime");
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
            strcpy(current->courseCode, coursecode);
            break;
        case 't':
            printf("Enter Time");
            strcpy(current->time, time);
            scanf("%s", time);
            break;
        default:
            printf("Enter Day: ");
            scanf("%s", day);
            strcpy(current->day, day);
            printf("Enter CourseCode");
            scanf("%s", coursecode);
            strcpy(current->courseCode, coursecode);
            printf("Enter Time");
            scanf("%s", time);
            scanf("%s", time);
    }

}

// void printLastChanges(struct Buildings *building) {}

//void revertChanges() {}

// void addRoom(struct Buildings *building) {}

// void addBuilding() {}

//  show aba
void deleteRoomSchedule(struct Rooms *room) {
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
               room->schedules[i].courseCode,
               room->schedules[i].time);
    }
}

// Transforms Uppercase words to lowercase
void upToLower(char word[10]) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = tolower(word[i]);
    }
}

void addRoomSchedule(struct Rooms* room) {
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
    upToLower(day);
    upToLower(coursecode);
    upToLower(time);

    printf("%s, %s, %s", day, coursecode, time);
    printf("%s", day);
    printf("   %s\n", DAYS[0]);

    // Checks if user inputted day exists on the DAYS array.
    for (int i = 0; i <= MAX_DAYS; i++) {
        if(i >= MAX_DAYS) {
            printf("Invalid day input, please try again.\n");
            addRoomSchedule(room);
        }
        if(strcmp(DAYS[i], day) == 0)
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
            sscanf(room->schedules[index].time, "%50[^-]-%d:%d", &schedSecHalf, &schedSecHalfMin);
            if((FHI == schedFirstHalf) || (FHI == schedSecHalf && FHMI < schedSecHalfMin)) {//e.g 7 and 7:30, then error
                printf("Error overlapping values\n");
                return;
            }
        }
    }

    // Below this shit are buggy ass error handling, will improve later
    int isFAM = (firstHalf[strlen(firstHalf) - 2] == 'a') ? 1 : 0; // 1 if AM 0 if PM
    int isSAM = (secondHalf[strlen(secondHalf) - 2] == 'a') ? 1 : 0; // 1 if AM 0 if PM

    printf("isfAm: %d\n", isFAM);
    printf("issAm: %d\n", isSAM);

    
    if(firstHalf[strlen(firstHalf) - 2] == secondHalf[strlen(secondHalf) - 2]) { 
        if(SHI < FHI) { // if it's AM or PM and Second half is higher than first half then error
            printf("3Invalid time values, Please try again\n");
            addRoomSchedule(room);
        }
    }
    // HH:mm; if HH if over 12 then error(this aint military time bruv), if HH is less than 6 then error there's no way someone's class starts at 5
    // HH:mm; if HH if over 10(cause u cant have classes at 11 en't u?) or less than 1 then error(this aint military time bruv)
    if(isFAM && isSAM) {
        if(FHI > 12 || FHI < 6 || SHI > 12 || SHI < 6) { 
            printf("1Invalid time values, please try again\n");
            addRoomSchedule(room);
        }
    } else if(!isFAM && !isSAM) {
        if (FHI > 10 || FHI < 1 || SHI > 10 || SHI < 1) {
            printf("11Invalid time values, please try again\n");
            addRoomSchedule(room);
        }
    }

    if(FHMI || SHMI) // if  hindi 0 ang value ng minutes
        if((FHMI > 59 || FHMI < 0) || (SHMI > 59 || SHMI < 0)) {
            printf("2Invalid time value, please try again.\n");
            addRoomSchedule(room);
        }
    
    // Store add new Datas to the array of Schedule structures of the current Room structure passed
    struct Schedule *sched = &room->schedules[room->scheduleCount++];
    strcpy(sched->day, day);
    strcpy(sched->courseCode, coursecode);
    strcpy(sched->time, time);

    for (int i = 0; i < room->scheduleCount; i++) 
        printf("%d.  %s, %s at %s\n",
               i,
               room->schedules[i].day,
               room->schedules[i].courseCode,
               room->schedules[i].time);
}
// Sort Schedules using the bubble sort Algo
void sortSchedules(struct Rooms* room) {
    // Sort schedules per day using bubble sort
    for (int i = 0; i < room->scheduleCount - 1; i++) {
        for (int k = 0; k < room->scheduleCount - i - 1; k++) {
            int dayIndex = 0, dayIndex2 = 0;
            
            // Find the index of the first day
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[k].day, DAYS[index]) == 0) {
                    dayIndex = index;
                    break;
                }
            }
            
            // Find the index of the second day
            for (int index = 0; index < MAX_DAYS; index++) {
                if (strcasecmp(room->schedules[k+1].day, DAYS[index]) == 0) {
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
            sscanf(room->schedules[k+1].time, "%d", &secondHalf);
            
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



int main() {
    // listOfBuildingsPointer, currentBuildingPointer
    FILE *LOBPtr, *CBPtr;  

    int bNumber, maxRooms;
    char line[100];
    char buildingText[100];
    int currentRoom = 0; // Room 1, or index zero
    char option;

    LOBPtr = fopen("./buildings/current_changes/listOfBuildings.txt", "rt");
    if (LOBPtr == NULL) {
        printf("Error opening file");
        return 1;
    }

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
        // fgets(line, sizeof(line), CBPtr);
        // sscanf(line, "Programs: %s")

        struct Buildings *building = createBuilding(bNumber, maxRooms);
        struct Rooms *room = NULL;
        while (fgets(line, sizeof(line), CBPtr)) {
            // If Room: is present on the string
            if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                sscanf(line, "Room: %d", &currentRoom);
                room = createRoom(currentRoom, building);
                continue;
            }

            if (strlen(line) <= 1)
                continue; // If empty line skip.

            int dayIndex;
            char courseCode[21], time[21];

            if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
                if (room)
                    addSchedule(room, dayIndex, courseCode, time);
        }
    }

    fclose(CBPtr);


    // Sort each schedules per buildings and rooms
    struct Buildings *currentBuilding = bHead;
    while (currentBuilding != NULL) {
        struct Rooms *currentRoom = currentBuilding->head;
        while (currentRoom != NULL) {
            sortSchedules(currentRoom);
            currentRoom = currentRoom->next;
        }
        currentBuilding = currentBuilding->next;
    }

    while(1) {
        printf("\n\nChoose [a]dd Schedule, [d]elete Schedule, [e]dit Schedule, [p]rint Room Schedule, [q]uit: ");
        scanf(" %c", &option);
        if(option == 'q') break;

        printBuildings();
        printf("Choose Building: \n");
        int bNum;
        scanf("%d", &bNum);
        struct Buildings* selectedBuilding = selectBuilding(bNum);
    
        printRooms(selectedBuilding);
        int roomOfChoice;
        printf("Enter Room of choice: ");
        scanf("%d", &roomOfChoice);
        struct Rooms* selectedRoom = selectRoom(roomOfChoice, selectedBuilding->head);

        // kulang pa ng edit
        switch (option) {
        case 'a':
            _saveLastChanges(selectedBuilding);
            addRoomSchedule(selectedRoom);
            _saveCurrentChanges(selectedBuilding);
            break;
        case 'd':
            _saveLastChanges(selectedBuilding);
            printSelectedRoom(selectedRoom);
            deleteRoomSchedule(selectedRoom);
            _saveCurrentChanges(selectedBuilding);
            break;
        case 'p':
            printSelectedRoom(selectedRoom);
            break;
        case 'e':
            _saveLastChanges(selectedBuilding);
            editRoomSchedule(selectedRoom);
            _saveCurrentChanges(selectedBuilding);
        }
    }
    return 0;
}
