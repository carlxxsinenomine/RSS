#include<stdio.h>
#include<string.h>
#include<stdlib.h>

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
};


struct Buildings {
    int buildingNumber;
    struct Rooms* head;
    struct Rooms* last;
    struct Buildings *prev;
    struct Buildings *next;
} *bHead=NULL, *bLast=NULL;

const char* DAYS[MAX_DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

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

void printRooms(struct Buildings *building) {
    if (building->head == NULL) {
        printf("No rooms in the list.\n");
        return;
    }

    struct Rooms *current = building->head;
    printf("Room List:\n");
    while (current != NULL) {
        printf("Room %d:\n", current->roomNumber);
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

struct Buildings* createBuilding(int buildingNumber) {
    struct Buildings* newBuilding = (struct Buildings *) malloc(sizeof(struct Buildings));
    if (!newBuilding) { // if new building is NULL
        printf("Memory allocation failed.");
        return NULL;
    }

    newBuilding->buildingNumber = buildingNumber;
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
    // strcpy(newRoom->schedule[roomNumber-1][0], day);
    // strcpy(newRoom->schedule[roomNumber-1][1], courseCode);
    // strcpy(newRoom->schedule[roomNumber-1][2], time);


    if (currentBuilding->head == NULL) {
        currentBuilding->head = currentBuilding->last = newRoom; // set last equal to newRoom and head equal to last
    } else {
        currentBuilding->last->next = newRoom; // points to newRoom
        newRoom->prev = currentBuilding->last;
        currentBuilding->last = newRoom;
    }
    return currentBuilding->last;
}
// Pumili ng room na imomodify e.g. print, or baguhin yung values
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

// iprint ung list ng selected na room
void printSelectedRoom(struct Buildings *building, struct Rooms* room) {
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

int main() {
    // listOfBuildingsPointer, currentBuildingPointer
    FILE *LOBPtr, *CBPtr;           

    int bNumber, maxRooms;
    char line[100];
    int currentRoom = 0; // Room 1, or index zero

    LOBPtr = fopen("listOfBuildings.txt", "rt");
    if (LOBPtr == NULL) {
        perror("Error opening file");
        return 1;
    }

    while(fgets(line, sizeof(line), LOBPtr)) {
        // Stores the Building file name
        char buildingText[100];
        sscanf(line, "%s", buildingText);
        printf("Current Building: %s\n", buildingText);

        CBPtr = fopen(buildingText, "rt");
        // fgets rineread nya each line of a text, ung max letters na pede nya maread depends on the size of bytes specified
        // sscanf hinahanap nya sa array ang format na inespecify mo. e.g. "Room: 1", tas format mo "Room: %d". mareread nya ung 1
        fgets(line, sizeof(line), CBPtr);
        sscanf(line, "Building No: %d", &bNumber);
        fgets(line, sizeof(line), CBPtr);
        sscanf(line, "Max Rooms: %d", &maxRooms);
        struct Buildings *building = createBuilding(bNumber);
        struct Rooms *room = NULL;
        while (fgets(line, sizeof(line), CBPtr))
        {
            // If Room: is present on the string
            if (strstr(line, "Room:"))
            { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                sscanf(line, "Room: %d", &currentRoom);
                room = createRoom(currentRoom, building);
                continue;
            }

            if (strlen(line) <= 1)
                continue; // If empty line skip.

            int dayIndex;
            char courseCode[21], time[21];

            if (sscanf(line, "%d, %20[^,], %20[^\n]", &dayIndex, courseCode, time) == 3) // == 3; if 3 values are read
            {
                if (room)
                {
                    addSchedule(room, dayIndex, courseCode, time);
                }
            }
        }
    }



    printBuildings();
    printf("Enter Building number: ");
    int bNum;
    scanf("%d", &bNum);
    struct Buildings* selectedBuilding = selectBuilding(bNum);

    printRooms(selectedBuilding);
    int roomOfChoice;
    printf("Enter Room of choice: ");
    scanf("%d", &roomOfChoice);
    struct Rooms* selectedRoom = selectRoom(roomOfChoice, selectedBuilding->head);
    printSelectedRoom(selectedBuilding, selectedRoom);






    fclose(CBPtr);

    return 0;
}