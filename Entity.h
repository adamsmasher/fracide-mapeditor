#ifndef ENTITY_H
#define ENTITY_H

typedef struct Entity_tag {
    UBYTE entityNum;
    UBYTE row;
    UBYTE col;
    UBYTE vramSlot;
    
    /* TODO: tags */
} Entity;

#endif