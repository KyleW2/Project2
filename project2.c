#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* constants defined for each possible command */
#define REQUEST 0
#define RELEASE 1
#define LIST_AVAILABLE 2
#define LIST_ASSIGNED 3
#define FIND 4

/* algorithms */
#define BESTFIT 0
#define FIRSTFIT 1
#define NEXTFIT 2
#define WORSTFIT 3

#define MAX_PROCESSES 10

typedef struct
{
    int command;
    char processName[10];
    int requiredMem;
} Command;

typedef struct
{
    char processName[10];
    int memoryUsed;
    int position;
} Process;

typedef struct
{
    Process processes[MAX_PROCESSES];
    int nextIndex;
    int algo;
    int totalSpace;
    int spaceAvailable;
} Memory;

int strToAlgo(char *algoName)
{
    if(strcmp(algoName, "BESTFIT") == 0)
    {
        return 0;
    }
    else if(strcmp(algoName, "FIRSTFIT") == 0)
    {
        return 1;
    }
    else if(strcmp(algoName, "NEXTFIT") == 0)
    {
        return 2;
    }
    else if(strcmp(algoName, "WORSTFIT") == 0)
    {
        return 3;
    }

    return -1;
}

/* uses fscanf() to find the total number of commands (ignoring comments) so commandArray doesnt have garbage values */
int getNumCommands(char *fileName)
{
    int commandCount = 0;

    FILE *filePointer = fopen(fileName, "r");
    char word[10];

    while(fscanf(filePointer, "%s ", word) != EOF)
    {
        if(strcmp(word, "REQUEST") == 0)
        {
            commandCount++;

            /* skips next 2 words */
            fscanf(filePointer, "%s ", word);
            fscanf(filePointer, "%s ", word);
        }
        else if(strcmp(word, "RELEASE") == 0 || strcmp(word, "LIST") == 0 || strcmp(word, "FIND") == 0)
        {
            commandCount++;

            /* skips next word */
            fscanf(filePointer, "%s ", word);
        }
    }

    fclose(filePointer);
    return commandCount;
}

/* uses fscanf() with format "%s " to get each word in file, then creates the corresponding command object and adds to array */
void parseCommands(char *fileName, Command *commandArray)
{
    int commandCount = 0;

    FILE *filePointer = fopen(fileName, "r");
    char word[10];
    char nullWord[10] = "NULL"; // for commands that dont require a process name (to avoid uninitilized garbage ╨@)

    while(fscanf(filePointer, "%s ", word) != EOF)
    {
        if(strcmp(word, "REQUEST") == 0)
        {
            commandArray[commandCount].command = REQUEST;
            fscanf(filePointer, "%s ", word);
            for(int i = 0; i < 10; i++)
                commandArray[commandCount].processName[i] = word[i];
            fscanf(filePointer, "%s ", word);
            commandArray[commandCount].requiredMem = (int)strtol(word, (char **)NULL, 10); // strtol returns a long from the given string in base 10
            commandCount++;
        }
        else if(strcmp(word, "RELEASE") == 0)
        {
            commandArray[commandCount].command = RELEASE;
            fscanf(filePointer, "%s ", word);
            for(int i = 0; i < 10; i++)
                commandArray[commandCount].processName[i] = word[i];
            commandArray[commandCount].requiredMem = 0;
            commandCount++;
        }
        else if(strcmp(word, "LIST") == 0)
        {
            fscanf(filePointer, "%s ", word);

            if(strcmp(word, "AVAILABLE") == 0)
            {
                commandArray[commandCount].command = LIST_AVAILABLE;
                for(int i = 0; i < 10; i++)
                    commandArray[commandCount].processName[i] = nullWord[i];
                commandArray[commandCount].requiredMem = 0;
            }
            else
            {
                commandArray[commandCount].command = LIST_ASSIGNED;
                for(int i = 0; i < 10; i++)
                    commandArray[commandCount].processName[i] = nullWord[i];
                commandArray[commandCount].requiredMem = 0;
            }
            commandCount++;
        }
        else if(strcmp(word, "FIND") == 0)
        {
            commandArray[commandCount].command = FIND;
            fscanf(filePointer, "%s ", word);
            for(int i = 0; i < 10; i++)
                commandArray[commandCount].processName[i] = word[i];
            commandArray[commandCount].requiredMem = 0;
            commandCount++;
        }
    }
    fclose(filePointer);
}

Process newHole(int mem, int pos)
{
    Process hole;
    memcpy(hole.processName, "HOLE", sizeof(hole.processName));
    hole.memoryUsed = mem;
    hole.position = pos;

    return hole;
}

/* if the next process after a hole is another hole, turn first hole
   into combinations and second hole in DEAD process */
void combineHoles(Memory memory)
{
    for(int i = 0; i < memory.nextIndex; i++)
    {
        if(strcmp(memory.processes[i].processName, "HOLE") == 0 && strcmp(memory.processes[i+1].processName, "HOLE") == 0)
        {
            memory.processes[i].memoryUsed = memory.processes[i].memoryUsed + memory.processes[i+1].memoryUsed;

            memcpy(memory.processes[i+1].processName, "DEAD", sizeof(memory.processes[i+1].processName));
            memory.processes[i+1].position = -1;
        }
    }
}

void holeToProcess(char newName[], int requiredMem, Memory memory, int i)
{
    int memDifference = memory.processes[i].memoryUsed - requiredMem;

    memcpy(memory.processes[i].processName, newName, sizeof(memory.processes[i].processName));
    memory.processes[i].memoryUsed = requiredMem;

    if(memDifference == 0)
    {
        return;
    }
    // if hole had left over space, make new hole with memDifference at position (old position + memory taken up)
    memory.processes[memory.nextIndex] = newHole(memDifference, memory.processes[i].position + requiredMem);
    memory.spaceAvailable = memory.spaceAvailable - requiredMem;
    
    combineHoles(memory);
}

void bestfit(char processName[], int requiredMem, Memory memory)
{
    int smallestSize = memory.totalSpace; // largest possible size
    int smallestIndex = -1;

    for(int i = 0; i < memory.nextIndex; i++)
    {
        if(strcmp(memory.processes[i].processName, "HOLE") == 0)
        {
            if(memory.processes[i].memoryUsed >= requiredMem && memory.processes[i].memoryUsed < smallestSize)
            {
                smallestSize = memory.processes[i].memoryUsed;
                smallestIndex = i;
            }
        }
    }

    if(smallestIndex > -1)
    {
        holeToProcess(processName, requiredMem, memory, smallestIndex);
        printf("ALLOCATED %s %d\n", processName, requiredMem);
        return;
    }
    else
    {
        printf("FAIL REQUEST %s %n\n", processName, requiredMem);
        return;
    }
}
void firstfit(char processName[], int requiredMem, Memory memory)
{
    printf("FAIL REQUEST %s %n\n", processName, requiredMem);
}
void nextfit(char processName[], int requiredMem, Memory memory)
{
    printf("FAIL REQUEST %s %n\n", processName, requiredMem);
}
void worstfit(char processName[], int requiredMem, Memory memory)
{
    printf("FAIL REQUEST %s %n\n", processName, requiredMem);
}

void request(char processName[], int requiredMem, Memory memory)
{
    if(memory.spaceAvailable > requiredMem)
    {
        printf("FAIL REQUEST %s %n\n", processName, requiredMem);
        return;
    }

    switch(memory.algo)
    {
        case BESTFIT:
            bestfit(processName, requiredMem, memory);
            break;
        case FIRSTFIT:
            firstfit(processName, requiredMem, memory);
            break;
        case NEXTFIT:
            nextfit(processName, requiredMem, memory);
            break;
        case WORSTFIT:
            worstfit(processName, requiredMem, memory);
            break;
    }
}

void release(char processName[], Memory memory)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(strcmp(memory.processes[i].processName, processName) == 0)
        {
            memcpy(memory.processes[i].processName, "HOLE", sizeof(memory.processes[i].processName));
            memory.spaceAvailable = memory.spaceAvailable + memory.processes[i].memoryUsed;
            printf("FREE %s %d %d\n", processName, memory.processes[i].memoryUsed, memory.processes[i].position);
            return;
        }
    }
    printf("FAIL RELEASE %s\n", processName);
    return;
}

void listAvailable(Memory memory)
{
    int available = 0;
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(strcmp(memory.processes[i].processName, "HOLE") == 0)
        {
            printf("(%d, %d) ", memory.processes[i].memoryUsed, memory.processes[i].position);
            available++;
        }
    }

    if(available > 0)
    {
        printf("\n");
        return;
    }
    else
    {
        printf("FULL\n");
        return;
    }
}

void listAssigned(Memory memory)
{
    int assigned = 0;
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(!(strcmp(memory.processes[i].processName, "HOLE") == 0) && !(strcmp(memory.processes[i].processName, "DEAD") == 0))
        {
            printf("(%s, %d, %d) ", memory.processes[i].processName, memory.processes[i].memoryUsed, memory.processes[i].position);
            assigned++;
        }
    }

    if(assigned > 0)
    {
        printf("\n");
        return;
    }
    else
    {
        printf("NONE\n");
        return;
    }
}

void find(char processName[], Memory memory)
{
    for(int i = 0; i < MAX_PROCESSES; i++)
    {
        if(strcmp(memory.processes[i].processName, processName) == 0)
        {
            printf("(%s, %d, %d)\n", processName, memory.processes[i].memoryUsed, memory.processes[i].position);
            return;
        }
    }
    printf("FAULT\n");
    return;
}

void run(Command command, Memory memory)
{
    switch(command.command)
    {
        case REQUEST:
            request(command.processName, command.requiredMem, memory);
            break;
        case RELEASE:
            release(command.processName, memory);
            break;
        case LIST_AVAILABLE:
            listAvailable(memory);
            break;
        case LIST_ASSIGNED:
            listAssigned(memory);
            break;
        case FIND:
            find(command.processName, memory);
            break;
        default:
            printf("BAD COMMAND\n");
            break;
    }
}

/* creates a hole process at the start of memory */
void initMemory(Memory memory)
{
    memory.processes[0] = newHole(memory.totalSpace, 0);
    memory.nextIndex = 1;
    memory.spaceAvailable = memory.totalSpace;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Not enough arguments.\n./project2 <algorithm> <total memory> <script>\n");
    }

    Memory memory;
    memory.algo = strToAlgo(argv[1]);
    memory.totalSpace = (int)pow(2, strtol(argv[2], (char **)NULL, 10));
    initMemory(memory);

    char *fileName = argv[3];

    int numOfCommands = getNumCommands(fileName);

    Command commandArray[numOfCommands];
    parseCommands(fileName, commandArray);

    for(int i = 0; i < numOfCommands; i++)
    {
        run(commandArray[i], memory);
    }

    return 0;
}