#ifndef ENTITY_H
#define ENTITY_H

#include <exec/types.h>

#define MAX_TAGS_PER_ENTITY 8

typedef struct Frac_tag_tag {
  UBYTE id;
  UBYTE value;
} Frac_tag;

typedef struct Entity_tag {
    UBYTE entityNum;
    UBYTE row;
    UBYTE col;
    UBYTE vramSlot;
    
    int tagCnt;
    Frac_tag tags[MAX_TAGS_PER_ENTITY];
} Entity;

#endif
