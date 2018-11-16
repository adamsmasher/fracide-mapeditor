#include "NumberedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <proto/exec.h>

static char *makeItemName(UWORD i, const char *string) {
  char *itemName = malloc(strlen(string) + 8);
  if(!itemName) {
    fprintf(stderr, "makeItemName: couldn't allocate item name\n");
    goto error;
  }

  sprintf(itemName, "%d: %s", i, string);
  return itemName;
error:
  return NULL;
}

struct List *newNumberedList(GetString getString, void *src, UWORD size) {
  struct List *list;
  struct Node *node, *next;
  int i;

  list = malloc(sizeof(struct List));
  if(!list) {
    fprintf(stderr, "newNumberedList: couldn't allocate list\n");
    goto error;
  }
  NewList(list);

  for(i = 0; i < size; i++) {
    const char *string;
    char *itemName;

    string = (*getString)(src, i);

    node = malloc(sizeof(struct Node));
    if(!node) {
      fprintf(stderr, "newNumberedList: couldn't allocate node\n");
      goto error_freeList;
    }

    itemName = makeItemName(i, string);
    if(!itemName) {
      fprintf(stderr, "newNumberedList: couldn't allocate uten name\n");
      goto error_freeNode;
    }

    node->ln_Name = itemName;
    AddTail(list, node);
    continue;
  error_freeNode:
    free(node);
    goto error_freeList;
  }

  return list;

error_freeList:
  node = list->lh_Head;
  while(next = node->ln_Succ) {
    free(node->ln_Name);
    free(node);
    node = next;
  }  
  free(list);
error:
  return NULL;
}

void freeNumberedList(struct List *list) {
  struct Node *node, *next;

  node = list->lh_Head;
  while(next = node->ln_Succ) {
    free(node->ln_Name);
    free(node);
    node = next;
  }

  free(list);
}

static struct Node *getItemNode(struct List *list, UWORD i) {
  struct Node *node, *next;

  node = list->lh_Head;
  while(i && (next = node->ln_Succ)) {
    node = next;
    i--;
  }

  if(node->ln_Succ) {
    return node;
  } else {
    return NULL;
  }
}

void numberedListSetItem(struct List *list, UWORD i, const char *string) {
  char *itemName;
  struct Node *node = getItemNode(list, i);
  if(!node) {
    fprintf(stderr, "numberedListSetItem: attempted to set invalid item\n");
    return;
  }

  itemName = makeItemName(i, string);
  if(!itemName) {
    fprintf(stderr, "numberedListSetItem: can't make item name\n");
    return;
  }

  free(node->ln_Name);
  node->ln_Name = itemName;
}
