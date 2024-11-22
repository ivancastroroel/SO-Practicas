#include "memoryList.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void initializeMemoryList(MemoryList *list)
{
    *list = NULL;
}

bool isMemoryListEmpty(MemoryList list)
{
    return (list == NULL);
}

MemoryNodePtr getFirstMemoryBlock(MemoryList list)
{
    return list;
}

MemoryNodePtr getLastMemoryBlock(MemoryList list)
{
    MemoryNodePtr p;

    p = list;
    if (p == NULL)
        return NULL;
    while (p->next != NULL)
        p = p->next;
    return p;
}

MemoryNodePtr getNextMemoryBlock(MemoryNodePtr node, MemoryList list)
{
    return node->next;
}

MemoryNodePtr getPreviousMemoryBlock(MemoryNodePtr node, MemoryList list)
{
    MemoryNodePtr temp;
    if (node == list)
        return NULL;
    else
    {
        temp = list;
        while (temp->next != node)
            temp = temp->next;

        return temp;
    }
}

bool addMemoryBlock(MemoryBlock block, MemoryList *list)
{
    MemoryNodePtr newNode, lastNode;

    // Allocate memory for a new node
    newNode = malloc(sizeof(*newNode));
    if (newNode == NULL)
    {
        perror("Error allocating memory for the new node");
        return false;
    }

    // Initialize the new node
    newNode->data = block;
    newNode->next = NULL;

    // Add the node to the list
    if (*list == NULL)
    {
        // If the list is empty, the new node becomes the first node
        *list = newNode;
    }
    else
    {
        // Traverse to the end of the list
        lastNode = *list;
        while (lastNode->next != NULL)
        {
            lastNode = lastNode->next;
        }
        lastNode->next = newNode;
    }

    return true; // Successfully added
}

void removeMemoryBlockAt(MemoryNodePtr node, MemoryList *list)
{
    MemoryNodePtr prevNode;

    if (node == *list)
        *list = (*list)->next;
    else if (node->next == NULL)
    {
        prevNode = getPreviousMemoryBlock(node, *list);
        prevNode->next = NULL;
    }
    else
    {
        prevNode = node->next;
        node->data = prevNode->data;
        node->next = prevNode->next;
        node = prevNode;
    }
    if (!strcmp(node->data.mode, "shared"))
    {
        if (shmdt(node->data.address))
            perror("Failed to detach shared memory: ");
    }
    if (!strcmp(node->data.mode, "malloc"))
        free(node->data.address);
    if (!strcmp(node->data.mode, "mmaped"))
    {
        munmap(node->data.address, node->data.size);
        close(node->data.fileDescriptor);
    }

    free(node);
}

MemoryBlock retrieveMemoryBlock(MemoryNodePtr node, MemoryList list)
{
    return node->data;
}

void updateMemoryBlock(MemoryBlock block, MemoryNodePtr node, MemoryList *list)
{
    node->data = block;
}

MemoryNodePtr searchMemoryBlock(memoryCommand command, MemoryList list)
{
    MemoryNodePtr current;

    for (current = list; (current != NULL) && (strcmp(current->data.address, command) < 0); current = current->next)
        ;
    if (current != NULL && (strcmp(current->data.address, command) == 0))
        return current;
    else
        return NULL;
}

void clearMemoryList(MemoryList *list)
{
    MemoryNodePtr current, temp;
    current = *list;

    if (*list == NULL)
        return;

    while (current != NULL)
    {
        temp = current;
        current = current->next;

        removeMemoryBlockAt(temp, list);
    }
    initializeMemoryList(list);
}

void displayMemoryList(MemoryList list, char *mode)
{
    MemoryNodePtr current;
    bool mmapMode = false, mallocMode = false, sharedMode = false, all = false;
    const char *months[12] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"};

    if (!strcmp("malloc", mode))
        mallocMode = true;
    else if (!strcmp("mmaped", mode))
        mmapMode = true;
    else if (!strcmp("shared", mode))
        sharedMode = true;
    else
        all = true;

    printf("******Memory blocks allocated (%s) for process %d******\n", mode, getpid());

    current = list;

    if (mallocMode)
    {
        while (current != NULL)
        {
            if (!strcmp(current->data.mode, mode))
                printf(" %10p%15d%5s%5d:%d %.15s\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, mode);
            current = current->next;
        }
    }
    else if (mmapMode)
    {
        while (current != NULL)
        {
            if (!strcmp(current->data.mode, mode))
                printf(" %10p%15d%5s%5d:%d%15s  (descriptor %d)\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, current->data.name, current->data.fileDescriptor);
            current = current->next;
        }
    }
    if (sharedMode)
    {
        while (current != NULL)
        {
            if (!strcmp(current->data.mode, mode))
                printf(" %10p%15d%5s%5d:%d%5s shared  (key %d)\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, current->data.name, current->data.key);
            current = current->next;
        }
    }
    else if (all)
    {
        while (current != NULL)
        {
            if (!strcmp(current->data.mode, "shared"))
                printf(" %10p%15d%5s%5d:%d%5s shared  (key %d)\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, current->data.name, current->data.key);
            if (!strcmp(current->data.mode, "malloc"))
                printf(" %10p%15d%5s%5d:%d%12s\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, "malloc");
            if (!strcmp(current->data.mode, "mmaped"))
                printf(" %10p%15d%5s%5d:%d%12s  (descriptor %d)\n", current->data.address, current->data.size, months[current->data.timestamp->tm_mon], current->data.timestamp->tm_hour, current->data.timestamp->tm_min, current->data.name, current->data.fileDescriptor);
            current = current->next;
        }
    }
}