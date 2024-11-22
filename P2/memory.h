#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "memoryList.h"
#include <errno.h>
#include <sys/shm.h>

#define MEMORY_BLOCK_SIZE 2048

void allocateMemory(int size, MemoryList *list);
void deallocateMemory(char *type, MemoryList *list, int size);
void displayMemoryMap(void);
void *getSharedMemorySegment(key_t key, size_t size, MemoryList *list);
void createSharedMemory(char *args, MemoryList *list);
void freeSharedMemory(int key, MemoryList *list);
void accessExistingSharedMemory(char *args, MemoryList *list);
void deleteSharedMemoryKey(char *args);
void mapFileMemory(char *filename, char *permissions, MemoryList *list);
void *mapFile(char *filename, int protection, MemoryList *list);
void releaseMappedFile(char *filename, MemoryList *list);
int readFromFile(char *filename, char *address, memoryCommand args);
int writeToFile(memoryCommand args);
int readFromFileDescriptor(int fd, char *address, memoryCommand args);
int writeToFileDescriptor(int fd, memoryCommand args);
int trocearCadena(memoryCommand input, char *segments[]);
void fillMemory(memoryCommand args);
void dumpMemory(memoryCommand args);
void recursiveFunction(int n);
void displayMemoryVariables();
void displayMemoryFunctions();

#endif