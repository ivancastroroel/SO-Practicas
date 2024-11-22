#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

#define COMMAND_LENGTH 2000

typedef char Command[COMMAND_LENGTH];

typedef struct ListItem
{
    Command name;
    int fileDescriptor;
    int posicion;
    int mode;
} ListItem;

typedef struct ListNode *NodePosition;
struct ListNode
{
    NodePosition next;
    ListItem data;
};
typedef NodePosition List;

void initializeList(List *list);

bool addItemToList(ListItem newItem, List *list);

void updateNodeData(ListItem updatedItem, NodePosition node, List *list);

void removeNodeAtPosition(NodePosition node, List *list);

bool isListEmpty(List list);

NodePosition getFirstNode(List list);

NodePosition getLastNode(List list);

NodePosition getNextNode(NodePosition node, List list);

NodePosition getPreviousNode(NodePosition node, List list);

ListItem getNodeData(NodePosition node, List list);

NodePosition findNodeByName(Command name, List list);

NodePosition findNodeByPosition(int position, List list);

NodePosition findNodeByDescriptor(int descriptor, List list);

#endif