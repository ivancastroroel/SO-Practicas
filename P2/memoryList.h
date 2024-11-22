#ifndef MEMORY_LIST_H_
#define MEMORY_LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MAX_COMMAND_LENGTH 2000

typedef char memoryCommand[MAX_COMMAND_LENGTH];

typedef struct MemoryBlock
{
    void *address;
    int size;
    struct tm *timestamp;
    memoryCommand mode;
    int key;
    int fileDescriptor;
    memoryCommand name;
} MemoryBlock;

typedef struct MemoryNode *MemoryNodePtr;
struct MemoryNode
{
    MemoryNodePtr next;
    MemoryBlock data;
};
typedef MemoryNodePtr MemoryList;

void initializeMemoryList(MemoryList *list);

bool addMemoryBlock(MemoryBlock block, MemoryList *list);

void updateMemoryBlock(MemoryBlock block, MemoryNodePtr node, MemoryList *list);

void removeMemoryBlockAt(MemoryNodePtr node, MemoryList *list);

bool isMemoryListEmpty(MemoryList list);

MemoryNodePtr getFirstMemoryBlock(MemoryList list);

MemoryNodePtr getLastMemoryBlock(MemoryList list);

MemoryNodePtr getNextMemoryBlock(MemoryNodePtr node, MemoryList list);

MemoryNodePtr getPreviousMemoryBlock(MemoryNodePtr node, MemoryList list);

MemoryBlock retrieveMemoryBlock(MemoryNodePtr node, MemoryList list);

MemoryNodePtr searchMemoryBlock(memoryCommand command, MemoryList list);

void clearMemoryList(MemoryList *list);

void displayMemoryList(MemoryList list, char *mode);

#endif