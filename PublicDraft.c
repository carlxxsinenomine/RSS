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
struct Rooms* createRoom(int roomNumber);
struct Rooms* selectRoom(int roomNumber);
void printSelectedRoom(struct Rooms* room);
void printRoomNumber();

int main() {
    //TODO 1: Change this to  FILE *listOfBuildings, roomFileName; lang
    FILE *fptr, *listOfBuildings;           

    int bNumber, maxRooms;
    char line[100], bLine[100];
    int currentRoom = 0; // Room 1, or index zero

    head = NULL;
    last = NULL;
    
    /**
    @note: alisin mona tong dalawa isang file lang ireread nya which is yung listOfBuildings.txt
    */
    //TODO 2: Read only from the listOfBuildings.txt
    listOfBuildings = fopen("listOfBuildings.txt", "rt");


    if (listOfBuildings == NULL) {
        perror("Error opening file");
        return 1;
    }

    // TODO 3: Gawa kang another while loop na nagreread line by line from the listOfBuildings.txt
    // Same ng while loop below
    while (fgets(bLine, sizeof(bLine), listOfBuildings)) {
        sscanf(bLine, "[^\n]", line);

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

        struct Rooms *room = NULL;

        while(fgets(line, sizeof(line), fptr)) {
            /**
            @note: Gayahin mo tong logic dito para malaman kung new  Building file na ung naka store sa bLine variable
            if new file  name na point to the next struct ng building structure
            */
           if (strlen(line) <= 1) continue;// If empty line skip.
        
            // If Room: is present on the string
            if (strstr(line, "Room:")) { // strstr hinahanap nya ung inespecify mo ssa params from an array. e.g. "Room:", hinahanap nya sa array ung Room:
                sscanf(line, "Room: %d", &currentRoom);
                room = createRoom(currentRoom);
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
    

    // TODO 4: After taking the name of the bld file from the listOfBuildings.txt create another fopen() and read from that file name
    /**
    @example: the contents of the listOfBuildings.txt ay ganto:
            Building2.txt
            Building4.txt
    @perform: after reading the first line (e.g. Building2.txt) gawa kang yan gagamitin mong pangread sa file:
            fopen(bLine, "rt"); // suppose na ung bLine is ung inistoran ng string "Building2.txt" na naread from listOfBuildings.txt
    */

    /**
    @note: after that the following code snippets below i think konti nalang babaguhin or dadagdagan, btw ung prev codes for reading from the Building
    file is nasa loob ng while loop ng pagread nung sa listOfBuildings.txt
    */
   
    printf("Rooms in building 4: \n"); //will put nalang another function para sa %d na maga specify if b4 or b2
    printRoomNumber();
    int roomOfChoice;
    printf("\nEnter room number to view schedule: ");
    scanf("%d", &roomOfChoice);
    struct Rooms* selectedRoom = selectRoom(roomOfChoice);
    printSelectedRoom(selectedRoom);

    fclose(listOfBuildings);

    return 0;
}

void printRoomNumber(){
    int i;

    struct Rooms *current = head; 
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

struct Rooms* createRoom(int roomNumber) {

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


    if (head == NULL) {
        head = last = newRoom; // set last equal to newRoom and head equal to last
    } else {
        last->next = newRoom; // points to newRoom
        newRoom->prev = last;
        last = newRoom;
    }
    return newRoom;
}
// Pumili ng room na imomodify e.g. print, or baguhin yung values
struct Rooms* selectRoom(int roomNumber) {
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
// iprint ung list ng selected na room
void printSelectedRoom(struct Rooms* room) {
    int currentRoomNumber = room->roomNumber;
    printf("Room Number: %d\n", currentRoomNumber);
    // printf("Room Sched Count: %d", room->scheduleCount);
    if (head == NULL) {
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
