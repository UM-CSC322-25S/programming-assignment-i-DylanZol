#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 127  





typedef enum {
    slip,    // numbers 1-85
    land,    // letters A-Z
    trailor, // license plate (str)
    storage, // number 1-50
    no_place // in case needed
} storage_types;

typedef union {
    int SlipNumber;
    char BayLetter;
    char TrailorTag[10];
    int StorageNumber;
} LocationInfo;

typedef struct {
    int Length;                
    storage_types storageType;
    LocationInfo details;
    double AmtOwed;
    char Name[MAX_NAME_LENGTH + 1];
} boat;

boat *boats[MAX_BOATS];
int NumBoats = 0;

storage_types ConvStrToPlace(char *str) // to go from user input -> enum type
{
    if (strcasecmp(str, "slip") == 0) //strcasecmp neglects case 
						// > 0 -> returns greater than
						// = 0 -> returns they are the same
						// < 0 -> returns less than
        return slip;
    else if (strcasecmp(str, "land") == 0)
        return land;
    else if (strcasecmp(str, "trailor") == 0)
        return trailor;
    else if (strcasecmp(str, "storage") == 0)
        return storage;
    else
        return no_place;
}

const char *ConvPlaceToStr(storage_types place) //enum type to str
{
    switch(place)
    {
        case slip:
            return "slip";
        case land:
            return "land";
        case trailor:
            return "trailor";
        case storage:
            return "storage";
        default:
            return "unknown";
    }
}

int compareBoatNames(const void *a, const void *b) // for qsort
{
    boat * const *boatA = a;
    boat * const *boatB = b;
    return strcasecmp((*boatA)->Name, (*boatB)->Name);
}

void UpdateMonth()
{
    for (int i = 0; i < NumBoats; i++)
    {
        boat *b = boats[i];   // makes it easier to use boats        
        double rate = 0.0;             

        switch(b->storageType)
        {
            case slip:
                rate = 12.50;
                break;
            case land:
                rate = 14.00;
                break;
            case trailor:
                rate = 25.00;
                break;
            case storage:
                rate = 11.20;
                break;
        }
        double monthly = rate * b->Length;  
        b->AmtOwed += monthly;
    }
    printf("Monthly charges have been added to individual bills.\n");
}

void AcceptPayment()
{
    char Name[MAX_NAME_LENGTH + 1];
    printf("Enter boat name: ");
    scanf(" %[^\n]", Name); // problem: skips user input
			    // fixed - was stopping at first space

    bool found = false;
    int idx = -1;
    for (int i = 0; i < NumBoats; i++)
    {
        if (strcasecmp(boats[i]->Name, Name) == 0)
        {
            found = true;
            idx = i;
            break;
        }
    }
    if (!found) {
        printf("No boat with that name\n");
        return;
    }
    double payment;
    printf("Selected Boat Amount Owed: %.2f\n", boats[idx]->AmtOwed);  
    printf("Enter Payment: ");
    scanf("%lf", &payment);   
    if (payment > boats[idx]->AmtOwed)
    {
        printf("The amount you have entered is more than the amount owed.\n");
        return;
    }
    boats[idx]->AmtOwed -= payment;
    printf("Payment complete. New amount owed: $%.2f\n", boats[idx]->AmtOwed);
}

void RemoveBoat()
{
    bool found = false;
    char Name[MAX_NAME_LENGTH + 1];  
    int pos = -1;

    printf("Enter name of the boat you would like to remove: ");
    scanf(" %[^\n]", Name);
    for (int i = 0; i < NumBoats; i++)
    {
        boat *b = boats[i];
        if (strcasecmp(b->Name, Name) == 0)
        {
            found = true;
            pos = i;
            break;
        }
    }
    if (!found)
    {
        printf("Boat not found, check spelling.\n");
        return;
    }
    else {
        free(boats[pos]); // free old memory
    }
    
    for (int i = pos; i < NumBoats - 1; i++) {
        boats[i] = boats[i + 1];
    }
    NumBoats--;
    printf("Removal Successful\n");
}


void PrintInventory()
{ //use quicksort
    qsort(boats, NumBoats, sizeof(boat *), compareBoatNames); 

    for (int i = 0; i < NumBoats; i++) {
        boat *b = boats[i];
        printf("Boat Name: %s\nBoat Length: %d\nStorage Type: %s\n", 
               b->Name, b->Length, ConvPlaceToStr(b->storageType));
        switch(b->storageType)
        {
            case slip:
                printf("Slip Number: %2d\n", b->details.SlipNumber);
                break;
            case land:
                printf("Bay Letter: %c\n", b->details.BayLetter);
                break;
            case trailor:
                printf("Trailor Tag: %s\n", b->details.TrailorTag);
                break;
            case storage:
                printf("Storage Number: %2d\n", b->details.StorageNumber);
                break;
            default:
                printf("Unknown Location, advise\n");
                break;
        }
        printf("Amount Owed: %.2f\n\n", b->AmtOwed);
    }
}

void AddBoat()
{
    char line[256];
    if (NumBoats >= MAX_BOATS)
    {
        printf("Marina is full, no more boats allowed.\n");
        return;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) //clears input buffer
        ; 

    printf("Enter boat information in CSV format (Name,Length,Type,LocationData,AmountOwed) (No Spaces): ");
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        printf("Error reading input.\n");
        return;
    }

    
    char name[MAX_NAME_LENGTH + 1];
    int length;
    char typeStr[20];
    char location[20];
    double amt;
    if (sscanf(line, "%[^,],%d,%[^,],%[^,],%lf", name, &length, typeStr, location, &amt) != 5)
    {
        printf("Invalid input format.\n");
        return;
    }

    boat *newBoat = (boat *)malloc(sizeof(boat));
    if (!newBoat)
    {
        printf("Memory Allocation failure\n");
        return;
    }
  
    strncpy(newBoat->Name, name, MAX_NAME_LENGTH);
    newBoat->Name[MAX_NAME_LENGTH] = '\0';  
    newBoat->Length = length;
    newBoat->AmtOwed = amt;
    newBoat->storageType = ConvStrToPlace(typeStr);

    switch(newBoat->storageType)
    {
        case slip:
            newBoat->details.SlipNumber = atoi(location);
            break;
        case land:
            newBoat->details.BayLetter = location[0];
            break;
        case storage:
            newBoat->details.StorageNumber = atoi(location);
            break;
        case trailor:
            strcpy(newBoat->details.TrailorTag, location);
            break;
        default:
            printf("Incorrect storage type.\n");
            free(newBoat);
            return;
    }
    int pos = 0;
    while (pos < NumBoats && strcasecmp(newBoat->Name, boats[pos]->Name) > 0)
        pos++;

    for (int i = NumBoats; i > pos; i--)
        boats[i] = boats[i-1];
    boats[pos] = newBoat;
    NumBoats++;
}



void LoadBoats(const char *filename) // populates boats array
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error could not open: %s\n", filename);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (NumBoats >= MAX_BOATS)
            break;
        boat *newBoat = (boat *)malloc(sizeof(boat));
        if (!newBoat)
        {
            printf("Memory Error\n");
            fclose(file);
            return;
        }
        char placeStr[20], locationData[20];
        if (sscanf(line, "%[^,],%d,%[^,],%[^,],%lf", newBoat->Name, &newBoat->Length, placeStr, locationData, &newBoat->AmtOwed) != 5) // parse csv
                       // reads up to comma 
	{
            printf("Error parsing line: %s\n", line);
            free(newBoat);
            continue;
        }
        newBoat->storageType = ConvStrToPlace(placeStr);
        switch(newBoat->storageType)  //set location data 
        {
            case slip:
                newBoat->details.SlipNumber = atoi(locationData);
                break;
            case land:
                newBoat->details.BayLetter = locationData[0];
                break;
            case trailor:
                strncpy(newBoat->details.TrailorTag, locationData, sizeof(newBoat->details.TrailorTag) - 1);
                newBoat->details.TrailorTag[sizeof(newBoat->details.TrailorTag) - 1] = '\0';
                break;
            case storage:
                newBoat->details.StorageNumber = atoi(locationData);  
                break;
            default:
                break;
        }
        boats[NumBoats++] = newBoat;
    }
    fclose(file);
}
void saveBoats(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Problem opening file.\n");
        exit(1);
    }
    for (int i = 0; i < NumBoats; i++) //writes boat info in csv
    {
        boat *b = boats[i];
        fprintf(file, "%s,%d,", b->Name, b->Length);
        switch(b->storageType)
        {
            case slip:
                fprintf(file, "slip,%d,", b->details.SlipNumber);
                break;
            case land:
                fprintf(file, "land,%c,", b->details.BayLetter);
                break;
            case trailor:
                fprintf(file, "trailor,%s,", b->details.TrailorTag);
                break;
            case storage:
                fprintf(file, "storage,%d,", b->details.StorageNumber);
                break;
            default:
                fprintf(file, "unknown,0,");
                break;
        }
        fprintf(file, "%.2f\n", b->AmtOwed);  
    }
    fclose(file);
}

int main(int argc, char *argv[]) //format for accepting argument from compile
{
    if (argc < 2)
    {
        printf("Incorrect file format\n");
        return 1;
    }
    LoadBoats(argv[1]); //load file in
    char option;

    printf("Welcome to the Boat Management System\n -------------------------------------\n");
    while (1)
    {
	
        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &option);
        switch(option)
        {
            case 'i':
            case 'I':
                PrintInventory();
                break;
            case 'a':
            case 'A':
                AddBoat();
                break;
            case 'r':
            case 'R':
                RemoveBoat();
                break;
            case 'p':
            case 'P':
                AcceptPayment();
                break;
            case 'm':
            case 'M':
                UpdateMonth();
                break;
            case 'x':
            case 'X':
                saveBoats(argv[1]);
                printf("Exiting the Boat Management System\n");
		for(int i=0;i<NumBoats;i++)
		  free(boats[i]);
                return 0;  
            default:
                printf("Invalid option, try again\n");
                break;
        }
    }
    return 0;
}
