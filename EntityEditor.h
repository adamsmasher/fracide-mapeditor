#ifndef ENTITY_EDITOR_H
#define ENTITY_EDITOR_H

#define ENTITY_EDITOR_LIST_ID (0)
#define ENTITY_NAME_ID (ENTITY_EDITOR_LIST_ID + 1)

typedef struct EntityEditor_tag {
    struct Window *window;
    struct Gadget *gadgets;
    struct Gadget *entityNameGadget;
    int closed;
    int selected;
    int editable;
} EntityEditor;

void initEntityEditorScreen(void);
void initEntityEditorVi(void);

EntityEditor *newEntityRequester(void);
EntityEditor *newEntityEditor(void);

void resizeEntityEditor(EntityEditor*);

void freeEntityEditor(EntityEditor*);

#endif
