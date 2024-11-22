#include "memory.h"
#include "memoryList.h"
#include <stdio.h>

int *varA, *varB, *varC, *varD, *varE, *varF;

int trocearCadena(memoryCommand commandInput, char *segments[])
{
    int index = 1;

    if ((segments[0] = strtok(commandInput, " \n\t")) == NULL)
        return 0;
    while ((segments[index] = strtok(NULL, " \n\t")) != NULL)
        index++;
    return index;
}

void allocateMemory(int blockSize, MemoryList *memoryList)
{
    MemoryBlock memoryBlock;
    struct tm *currentTimestamp;
    time_t currentTime;

    time(&currentTime);
    currentTimestamp = localtime(&currentTime);

    memoryBlock.address = malloc(blockSize);
    if (memoryBlock.address == NULL)
    {
        perror("Failed to allocate memory");
        return;
    }

    memoryBlock.size = blockSize;
    strcpy(memoryBlock.mode, "malloc");
    memoryBlock.timestamp = currentTimestamp;
    memoryBlock.fileDescriptor = -1;
    memoryBlock.key = -1;
    strcpy(memoryBlock.name, "");

    addMemoryBlock(memoryBlock, memoryList);
    printf("Allocated %d bytes at %p\n", blockSize, memoryBlock.address);
}

void deallocateMemory(char *memoryType, MemoryList *memoryList, int blockSize)
{
    MemoryNodePtr currentNode;

    currentNode = *memoryList;

    while (currentNode != NULL)
    {
        if (currentNode->data.size == blockSize && !strcmp(currentNode->data.mode, "malloc"))
        {
            removeMemoryBlockAt(currentNode, memoryList);
            return;
        }
        currentNode = currentNode->next;
    }
    printf("No block of this size found\n");
}

void displayMemoryMap(void) /* Sin argumentos */
{
    pid_t processId; /* Muestra el mapa de memoria del proceso actual */
    char processIdString[32];
    char *commandArgs[4] = {"pmap", processIdString, NULL};

    sprintf(processIdString, "%d", (int)getpid());
    if ((processId = fork()) == -1)
    {
        perror("No se pudo crear el proceso hijo");
        return;
    }
    if (processId == 0)
    { /* Proceso hijo */
        if (execvp(commandArgs[0], commandArgs) == -1)
            perror("No se pudo ejecutar pmap (linux, solaris)");

        commandArgs[0] = "vmmap";
        commandArgs[1] = "-interleave";
        commandArgs[2] = processIdString;
        commandArgs[3] = NULL;
        if (execvp(commandArgs[0], commandArgs) == -1) /* Intentamos vmmap en Mac-OS */
            perror("No se pudo ejecutar vmmap (Mac-OS)");

        commandArgs[0] = "procstat";
        commandArgs[1] = "vm";
        commandArgs[2] = processIdString;
        commandArgs[3] = NULL;
        if (execvp(commandArgs[0], commandArgs) == -1) /* Intentamos procstat en FreeBSD */
            perror("No se pudo ejecutar procstat (FreeBSD)");

        commandArgs[0] = "procmap";
        commandArgs[1] = processIdString;
        commandArgs[2] = NULL;
        if (execvp(commandArgs[0], commandArgs) == -1) /* Intentamos procmap en OpenBSD */
            perror("No se pudo ejecutar procmap (OpenBSD)");

        exit(1);
    }
    waitpid(processId, NULL, 0);
}

void *getSharedMemorySegment(key_t memoryKey, size_t memorySize, MemoryList *memoryList)
{
    void *mappedMemory;
    int errorCode, segmentId, permissions = 0777;
    struct shmid_ds segmentInfo;
    MemoryBlock memoryBlock;
    struct tm *timestamp;
    time_t currentTime;

    time(&currentTime);
    timestamp = localtime(&currentTime);

    if (memorySize) /* Si el tamaño es distinto de 0, se debe crear */
        permissions |= IPC_CREAT | IPC_EXCL;

    if (memoryKey == IPC_PRIVATE) /* Claves privadas no son válidas */
    {
        errno = EINVAL;
        return NULL;
    }

    if ((segmentId = shmget(memoryKey, memorySize, permissions)) == -1)
        return NULL;

    if ((mappedMemory = shmat(segmentId, NULL, 0)) == (void *)-1)
    {
        errorCode = errno;
        if (memorySize)
            shmctl(segmentId, IPC_RMID, NULL);
        errno = errorCode;
        return NULL;
    }

    shmctl(segmentId, IPC_STAT, &segmentInfo);

    memoryBlock.key = memoryKey;
    memoryBlock.size = segmentInfo.shm_segsz;
    memoryBlock.fileDescriptor = -1;
    memoryBlock.timestamp = timestamp;
    memoryBlock.address = mappedMemory;
    strcpy(memoryBlock.mode, "shared");
    strcpy(memoryBlock.name, "");

    addMemoryBlock(memoryBlock, memoryList);

    return mappedMemory;
}

void accessExistingSharedMemory(char *params, MemoryList *memoryList)
{
    key_t key;
    void *sharedMemoryAddress;
    char *tokens[MAX_COMMAND_LENGTH];

    // Dividir los argumentos y obtener la clave
    trocearCadena(params, tokens);

    if (tokens[1] == NULL)
    {
        printf("Usage: -shared <key>\n");
        return;
    }

    key = (key_t)strtoul(tokens[2], NULL, 10);

    // Intentar acceder al segmento existente
    if ((sharedMemoryAddress = getSharedMemorySegment(key, 0, memoryList)) != NULL) // tam=0, accede a segmento existente
    {
        printf("Shared memory assigned:\n");
        printf("  Key: %lu\n", (unsigned long)key);
        printf("  Address: %p\n", sharedMemoryAddress);
    }
    else
    {
        printf("Error accessing shared memory. Key %lu: %s\n", (unsigned long)key, strerror(errno));
    }
}

void freeSharedMemory(int memoryKey, MemoryList *memoryList)
{
    MemoryNodePtr currentNode = *memoryList;

    while (currentNode->data.key != memoryKey)
    {
        currentNode = currentNode->next;
    }

    removeMemoryBlockAt(currentNode, memoryList);
}

void deleteSharedMemoryKey(char *arguments)
{
    key_t sharedKey;
    int sharedSegmentId;
    char *parsedKey, *segments[MAX_COMMAND_LENGTH];

    trocearCadena(arguments, segments);

    parsedKey = segments[2];

    if (parsedKey == NULL || (sharedKey = (key_t)strtoul(parsedKey, NULL, 10)) == IPC_PRIVATE)
    {
        printf("deleteSharedKey requires a valid key\n");
        return;
    }

    if ((sharedSegmentId = shmget(sharedKey, 0, 0666)) == -1)
    {
        perror("shmget: Unable to get shared memory");
        return;
    }

    if (shmctl(sharedSegmentId, IPC_RMID, NULL) == -1)
    {
        perror("shmctl: Unable to delete shared memory key\n");
    }
}

void createSharedMemory(char *arguments, MemoryList *memoryList)
{
    key_t key;
    size_t size;
    void *sharedMemoryAddress;
    char *tokens[MAX_COMMAND_LENGTH];

    trocearCadena(arguments, tokens);

    if (tokens[2] == NULL || tokens[3] == NULL)
    {
        fprintf(stderr, "Usage: shared -create <key> <size>\n");
        return;
    }

    key = (key_t)strtoul(tokens[2], NULL, 10);
    size = (size_t)strtoul(tokens[3], NULL, 10);

    if (size == 0)
    {
        fprintf(stderr, "Error: Cannot allocate memory blocks of size 0.\n");
        return;
    }

    // Attempt to allocate shared memory
    sharedMemoryAddress = getSharedMemorySegment(key, size, memoryList);
    if (sharedMemoryAddress != NULL)
    {
        printf("Allocated %lu bytes at %p\n", (unsigned long)size, sharedMemoryAddress);
    }
    else
    {
        fprintf(stderr, "Error allocating shared memory with key %lu: %s\n", (unsigned long)key, strerror(errno));
    }
}

void *mapFile(char *fileName, int accessMode, MemoryList *memoryList)
{
    int fileDescriptor, mappingType = MAP_PRIVATE, openMode = O_RDONLY;
    struct stat fileStats;
    void *mappedAddress;
    MemoryBlock memoryBlock;
    struct tm *currentTime;
    time_t rawTime;

    time(&rawTime);
    currentTime = localtime(&rawTime);

    if (accessMode & PROT_WRITE)
        openMode = O_RDWR;
    if (stat(fileName, &fileStats) == -1 || (fileDescriptor = open(fileName, openMode)) == -1)
        return NULL;
    if ((mappedAddress = mmap(NULL, fileStats.st_size, accessMode, mappingType, fileDescriptor, 0)) == MAP_FAILED)
        return NULL;

    memoryBlock.timestamp = currentTime;
    memoryBlock.address = mappedAddress;
    strcpy(memoryBlock.mode, "mmaped");
    memoryBlock.size = fileStats.st_size;
    memoryBlock.fileDescriptor = fileDescriptor;
    strcpy(memoryBlock.name, fileName);

    addMemoryBlock(memoryBlock, memoryList);
    return mappedAddress;
}

void mapFileMemory(char *filePath, char *permissions, MemoryList *memoryList)
{
    char *perm;
    void *mappedFileAddress;
    int accessMode = 0;

    if ((perm = permissions) != NULL && strlen(perm) < 4)
    {
        if (strchr(perm, 'r') != NULL)
            accessMode |= PROT_READ;
        if (strchr(perm, 'w') != NULL)
            accessMode |= PROT_WRITE;
        if (strchr(perm, 'x') != NULL)
            accessMode |= PROT_EXEC;
    }
    if ((mappedFileAddress = mapFile(filePath, accessMode, memoryList)) == NULL)
        perror("Unable to map file");
    else
        printf("File %s mapped to %p\n", filePath, mappedFileAddress);
}

void releaseMappedFile(char *fileName, MemoryList *memoryList)
{
    MemoryNodePtr currentNode = *memoryList;

    while (strcmp(currentNode->data.name, fileName))
        currentNode = currentNode->next;

    if (munmap(currentNode->data.address, currentNode->data.size))
    {
        perror("File not mapped");
        return;
    }
    close(currentNode->data.fileDescriptor);
    removeMemoryBlockAt(currentNode, memoryList);
}

int readFromFile(char *fileName, char *memoryAddress, memoryCommand args)
{
    struct stat fileStats;
    ssize_t bytesRead, totalBytesToRead;
    int fileDescriptor;
    void *targetAddress;
    char *parsedArgs[MAX_COMMAND_LENGTH];

    trocearCadena(args, parsedArgs);
    sscanf(memoryAddress, "0x%p", &targetAddress);

    if (parsedArgs[3] == NULL)
        totalBytesToRead = -1;
    else
        totalBytesToRead = atoi(parsedArgs[3]);

    if (stat(fileName, &fileStats) == -1 || (fileDescriptor = open(fileName, O_RDONLY)) == -1)
    {
        perror("Unable to read file: ");
        return -1;
    }
    if (totalBytesToRead == -1) /* If -1 is passed as bytes to read, read the whole file */
        totalBytesToRead = fileStats.st_size;
    if ((bytesRead = read(fileDescriptor, targetAddress, totalBytesToRead)) == -1)
    {
        perror("Unable to read file: ");
        close(fileDescriptor);
        return -1;
    }
    printf("Read %ld bytes from %s to %p\n", bytesRead, fileName, targetAddress);
    close(fileDescriptor);
    return 0;
}

int readFromFileDescriptor(int fileDescriptor, char *address, memoryCommand commandArgs)
{
    ssize_t bytesRead, bytesToRead;
    void *targetAddress;
    char *parsedArgs[MAX_COMMAND_LENGTH];

    // Dividir argumentos
    trocearCadena(commandArgs, parsedArgs);
    sscanf(address, "0x%p", &targetAddress);

    // Determinar la cantidad de bytes a leer
    if (parsedArgs[3] == NULL) {
        printf("Error: Falta el parámetro 'bytesToRead'.\n");
        return -1;
    }
    bytesToRead = atoi(parsedArgs[3]);

    if (bytesToRead <= 0) {
        printf("Error: El número de bytes a leer debe ser mayor que 0.\n");
        return -1;
    }

    // Leer del descriptor
    if ((bytesRead = read(fileDescriptor, targetAddress, bytesToRead)) == -1) {
        perror("No se pudo leer del descriptor de fichero");
        return -1;
    }

    printf("Leídos %ld bytes desde el descriptor %d en %p\n", bytesRead, fileDescriptor, targetAddress);
    return 0;
}

int writeToFile(memoryCommand commandArgs)
{
    ssize_t bytesWritten, bytesToWrite;
    int fileDescriptor, fileFlags = O_CREAT | O_EXCL | O_WRONLY;
    void *sourceAddress;
    char *parsedArgs[MAX_COMMAND_LENGTH], filePath[MAX_COMMAND_LENGTH];
    int argumentCount = trocearCadena(commandArgs, parsedArgs);

    if (argumentCount < 4 || (!strcmp(parsedArgs[1], "-o") && argumentCount < 5))
    {
        printf("Faltan parámetros\n");
        return -1;
    }

    if (!strcmp(parsedArgs[1], "-o"))
    {
        fileFlags = O_CREAT | O_WRONLY | O_TRUNC;
        bytesToWrite = atoi(parsedArgs[4]);
        sscanf(parsedArgs[3], "0x%p", &sourceAddress);
        strcpy(filePath, parsedArgs[2]);
    }
    else
    {
        bytesToWrite = atoi(parsedArgs[3]);
        sscanf(parsedArgs[2], "0x%p", &sourceAddress);
        strcpy(filePath, parsedArgs[1]);
    }

    if ((fileDescriptor = open(filePath, fileFlags, 0777)) == -1)
    {
        perror("No se pudo abrir el fichero para escritura: ");
        return -1;
    }

    if ((bytesWritten = write(fileDescriptor, sourceAddress, bytesToWrite)) == -1)
    {
        perror("No se pudo escribir en el fichero: ");
        close(fileDescriptor);
        return -1;
    }

    printf("Escritos %ld bytes en %s desde %p\n", bytesWritten, filePath, sourceAddress);
    close(fileDescriptor);
    return 0;
}

int writeToFileDescriptor(int fileDescriptor, memoryCommand commandArgs)
{
    ssize_t bytesWritten, bytesToWrite;
    void *sourceAddress;
    char *parsedArgs[MAX_COMMAND_LENGTH];
    int argumentCount = trocearCadena(commandArgs, parsedArgs);

    if (argumentCount < 3) {
        printf("Faltan parámetros\n");
        return -1;
    }

    // Obtener la dirección y la cantidad de bytes a escribir
    sscanf(parsedArgs[2], "0x%p", &sourceAddress);
    bytesToWrite = atoi(parsedArgs[3]);

    if (bytesToWrite <= 0) {
        printf("Error: El número de bytes a escribir debe ser mayor que 0.\n");
        return -1;
    }

    // Escribir en el descriptor
    if ((bytesWritten = write(fileDescriptor, sourceAddress, bytesToWrite)) == -1) {
        perror("No se pudo escribir en el descriptor de fichero");
        return -1;
    }

    printf("Escritos %ld bytes en el descriptor %d desde %p\n", bytesWritten, fileDescriptor, sourceAddress);
    return 0;
}

void fillMemory(memoryCommand commandArgs)
{
    void *targetAddress;
    unsigned char fillByte;
    char *parsedArgs[MAX_COMMAND_LENGTH];
    int argumentCount = trocearCadena(commandArgs, parsedArgs);

    if (argumentCount != 4)
    {
        printf("Parámetros inadecuados\n");
        return;
    }

    ssize_t byteCount = atoi(parsedArgs[2]), i;

    sscanf(parsedArgs[1], "0x%p", &targetAddress);
    sscanf(parsedArgs[3], "%hhu", &fillByte);
    unsigned char *byteArray = (unsigned char *)targetAddress;

    for (i = 0; i < byteCount; i++)
        byteArray[i] = fillByte;

    printf("Llenando %ld bytes de memoria con el byte (%hhu) a partir de la dirección %p\n", byteCount, fillByte, targetAddress);
}

void dumpMemory(memoryCommand commandArgs)
{
    void *sourceAddress;
    char *parsedArgs[MAX_COMMAND_LENGTH];
    int argumentCount = trocearCadena(commandArgs, parsedArgs);

    if (argumentCount != 3)
    {
        printf("Parámetros inadecuados\n");
        return;
    }

    ssize_t byteCount = atoi(parsedArgs[2]), i;

    sscanf(parsedArgs[1], "0x%p", &sourceAddress);
    unsigned char *byteArray = (unsigned char *)sourceAddress;

    printf("Volcando %lu bytes desde la dirección %p\n", byteCount, sourceAddress);
    for (i = 0; i < byteCount; i++)
    {
        printf("%hhu ", byteArray[i]);
    }
    puts("");
}

void recursiveFunction(int level)
{
    char autoArray[MEMORY_BLOCK_SIZE];
    static char staticArray[MEMORY_BLOCK_SIZE];

    printf("Nivel: %3d (%p) autoArray %p, staticArray %p\n", level, &level, autoArray, staticArray);

    if (level > 0)
        recursiveFunction(level - 1);
}

void displayMemoryVariables()
{
    int *local1 = malloc(sizeof(local1)), *local2 = malloc(sizeof(local2)), *local3 = malloc(sizeof(local3));
    int stackVar1, stackVar2, stackVar3, staticVar1 = 1, staticVar2 = 2, staticVar3 = 3;
    int *global1 = malloc(sizeof(global1));
    int *global2 = malloc(sizeof(global2));
    int *global3 = malloc(sizeof(global3));

    printf("%15s\t%15p, %15p, %15p\n", "Variables locales", &local1, &local2, &local3);
    printf("%15s\t%15p, %15p, %15p\n", "Variables globales", &global1, &global2, &global3);
    printf("%15s\t%15p, %15p, %15p\n", "Variables apiladas", &stackVar1, &stackVar2, &stackVar3);
    printf("%15s\t%15p, %15p, %15p\n", "Variables estáticas", &staticVar1, &staticVar2, &staticVar3);

    free(local1);
    free(local2);
    free(local3);
    free(global1);
    free(global2);
    free(global3);
}

void displayMemoryFunctions()
{
    printf("%15s\t%15p, %15p, %15p\n", "Funciones del programa", (void *)recursiveFunction, (void *)displayMemoryVariables, (void *)dumpMemory);
    printf("%15s\t%15p, %15p, %15p\n", "Funciones de librería", (void *)strcmp, (void *)malloc, (void *)getpid);
}