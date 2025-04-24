#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAXCOL 3
#define MAX_SCHEDULES 20
#define MAX_DAYS 5

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

const char* DAYS[MAX_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

void printRooms();
void addSchedule(struct Rooms *room, int dayIndex, const char *courseCode, const char *time);
struct Rooms* createRoom(struct Building *building, int roomNumber);
struct Rooms* selectRoom(struct Building *building, int roomNumber);
void printSelectedRoom(struct Building *building, struct Rooms* room, char *collegeProgram);
void printRoomNumber(struct Building *building);
struct Building *createBuilding(int bldgNum);
struct Building *selectBuilding(int bldgNum); 
void printfSelectedBuilding(struct Building *building);
void upToLower(char word[10]);
void printBuildingNumber();
int filterCollegeProgram(char *courseCode, char *college_program);

int main(void) {
  
    FILE *fptr, *listOfBuildingsPtr;           

    int bNumber, maxRooms;
    char line[100], bLine[100];
    int currentRoom = 0; // Room 1, or index zero

    head = NULL;
    last = NULL;
    
    listOfBuildingsPtr = fopen("listOfBuildings.txt", "rt");

    if (listOfBuildingsPtr == NULL) {
        perror("Error opening file");
        return 1;
    }

    // while loop na nagreread line by line from the listOfBuildings.txt
    while (fscanf(listOfBuildingsPtr, "%s", bLine) != EOF) {
       
        fptr = fopen(bLine, "rt");

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
   
    printBuildingNumber();
    char college_program[10];
    printf("Enter your college program: ");
    scanf("%s", college_program);
    upToLower(college_program);

    int buildingChoice;
    printf("Enter building number to view available rooms: ");
    scanf("%d", &buildingChoice);

    struct Building *selectedBuilding = selectBuilding(buildingChoice);
    printfSelectedBuilding(selectedBuilding);

    printRoomNumber(selectedBuilding);
    
    int roomOfChoice;
    printf("\nEnter room number to view schedule: ");
    scanf("%d", &roomOfChoice);

    struct Rooms* selectedRoom = selectRoom(selectedBuilding, roomOfChoice);
    printSelectedRoom(selectedBuilding, selectedRoom, NULL);

    char userChoice;
    printf("\nSort by college program [y/n]: ");
    scanf(" %c", &userChoice);

    switch (userChoice) {
    case 'Y':
    case 'y':
        printSelectedRoom(selectedBuilding, selectedRoom, college_program);
        break;
    case 'N':
    case 'n':
        printSelectedRoom(selectedBuilding, selectedRoom, NULL);
        break;
    
    default:
        printf("Invalid input");
        break;
    }

    fclose(listOfBuildingsPtr);

    return 0;
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

struct Building *selectBuilding(int bldgNum) {
    struct Building* current = bldgHead;
    while (current != NULL) {
        if (current->buildingNumber == bldgNum) { // if val of current->buildingNumber is equal to current edi same room
            return current; // return the current room na imomodify
        }

        if (current->next == NULL) {
            printf("Invalid building number.");
            return NULL;
        }
        current = current->next; // Iterate through the next List
    }
    return NULL;
}

void printfSelectedBuilding(struct Building *building){
    int currentBuildingNumber = building->buildingNumber;
    printf("\nRooms in building %d\n", currentBuildingNumber);

    if(bldgHead == NULL){
        printf("No available buildings");
        return;
    }
}

//date added: 04/15
void printBuildingNumber() {
    struct Building *current = bldgHead;
    printf("Available buildings: \n");

    while(current != NULL) {
        printf("Building %d\n", current->buildingNumber);
        current = current->next;
    }
}

//date edited: 04/15
void printRoomNumber(struct Building *building){
    struct Rooms *current = building->head; 

    while(current != NULL) {
        printf("Room %d\n", current->roomNumber);
        current = current->next;
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
    // strcpy(newRoom->schedule[roomNumber-1][0], day);
    // strcpy(newRoom->schedule[roomNumber-1][1], courseCode);
    // strcpy(newRoom->schedule[roomNumber-1][2], time);

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
void printSelectedRoom(struct Building *building, struct Rooms* room, char *collegeProgram) {
    int currentRoomNumber = room->roomNumber;
    printf("Room Number: %d\n", currentRoomNumber);
    // printf("Room Sched Count: %d", room->scheduleCount);
    if (building->head == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    int flag = 0;
    for (int i = 0; i < room->scheduleCount; i++) {
        if(collegeProgram == NULL||filterCollegeProgram(room->schedules[i].courseCode, collegeProgram)){
            printf("  %s, %s at %s\n",
               room->schedules[i].day,
               room->schedules[i].courseCode,
               room->schedules[i].time);
            flag = 1;
        }
    }
    if(flag == 0){
        printf("No schedule for %s", collegeProgram);
    }
}

//date added: 04/23
//date edited: 04/24
void upToLower(char word[10]) {
    for(int i = 0; i< strlen(word); i++) word[i] = tolower(word[i]);
}

//date added: 04/23
//date edited: 04/24
int filterCollegeProgram(char *courseCode, char *college_program) {
    return strstr(courseCode, college_program) != NULL;
}
