#ifndef ENTITY_REQUESTER_H
#define ENTITY_REQUESTER_H

#define ENTITY_REQUESTER_LIST_ID (0)
#define ENTITY_NAME_ID (ENTITY_REQUESTER_LIST_ID + 1)

typedef struct EntityRequester_tag {
    struct Window *window;
    struct Gadget *gadgets;
    struct Gadget *entityNameGadget;
    int closed;
    int selected;
    int editable;
} EntityRequester;

void initEntityRequesterScreen(void);
void initEntityRequesterVi(void);

EntityRequester *newEntityRequester(void);
EntityRequester *newEntityEditor(void);

void resizeEntityRequester(EntityRequester*);

void freeEntityRequester(EntityRequester*);

#endif
