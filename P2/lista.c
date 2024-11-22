#include "lista.h"
#include <string.h>

void initializeList(List *list)
{
    *list = NULL;
}

bool isListEmpty(List list)
{
    return (list == NULL);
}

NodePosition getFirstNode(List list)
{
    return list;
}

NodePosition getLastNode(List list)
{
    NodePosition currentNode;

    currentNode = list;
    if (currentNode == NULL)
        return NULL;
    while (currentNode->next != NULL)
        currentNode = currentNode->next;
    return currentNode;
}

NodePosition getNextNode(NodePosition node, List list)
{
    return node->next;
}

NodePosition getPreviousNode(NodePosition node, List list)
{
    NodePosition currentNode;
    if (node == list)
        return NULL;
    else
    {
        currentNode = list;
        while (currentNode->next != node)
            currentNode = currentNode->next;

        return currentNode;
    }
}

bool addItemToList(ListItem newItem, List *list)
{
    NodePosition newNode, currentNode;

    newNode = malloc(sizeof(*newNode));

    if (newNode == NULL)
        return false;

    newNode->data = newItem;
    newNode->next = NULL;

    if (*list == NULL)
    {
        newNode->data.posicion = 0;
        *list = newNode;
    }
    else
    {
        currentNode = *list;
        while (currentNode->next != NULL)
            currentNode = currentNode->next;

        newNode->data.posicion = currentNode->data.posicion + 1;
        currentNode->next = newNode;
    }
    return true;
}

void removeNodeAtPosition(NodePosition node, List *list)
{
    NodePosition previousNode;

    if (node == *list) // Node is at the head of the list
        *list = (*list)->next;
    else if (node->next == NULL) // Node is the last in the list
    {
        previousNode = getPreviousNode(node, *list);
        previousNode->next = NULL;
    }
    else // Node is in the middle of the list
    {
        previousNode = node->next;
        node->data = previousNode->data;
        node->next = previousNode->next;
        node = previousNode;
    }
    free(node);
}

ListItem getNodeData(NodePosition node, List list)
{
    return node->data;
}

void updateNodeData(ListItem updatedItem, NodePosition node, List *list)
{
    node->data = updatedItem;
}

NodePosition findNodeByName(Command name, List list)
{
    NodePosition currentNode;

    for (currentNode = list; (currentNode != NULL) && (strcmp(currentNode->data.name, name) < 0); currentNode = currentNode->next)
        ;
    if (currentNode != NULL &&
        (strcmp(currentNode->data.name, name) == 0))
        return currentNode;
    else
        return NULL;
}

NodePosition findNodeByPosition(int position, List list)
{
    NodePosition currentNode;

    currentNode = list;
    if (currentNode == NULL)
        return currentNode;

    while (currentNode->data.posicion != position && currentNode->next != NULL)
        currentNode = currentNode->next;

    if (position == currentNode->data.posicion)
        return currentNode;
    else
        return NULL;
}

NodePosition findNodeByDescriptor(int descriptor, List list)
{
    NodePosition currentNode, tempNode;

    currentNode = list;
    if (currentNode == NULL)
        return currentNode;

    while (currentNode->data.fileDescriptor != descriptor && currentNode->next != NULL)
        currentNode = currentNode->next;

    if (descriptor == currentNode->data.fileDescriptor)
        return currentNode;

    else
    {
        tempNode = malloc(sizeof(*tempNode)); // Allocate memory to return a temporary node
        tempNode->data.fileDescriptor = (-1);
        return tempNode;
    }
}