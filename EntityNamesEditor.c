#include "EntityNamesEditor.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include "EntityRequester.h"

static BOOL isEntityNamesEditor(FrameworkWindow *window) {
  if(isEntityRequester(window)) {
    EntityRequesterData *data = window->data;
    return (BOOL)data->editable;
  }
  return FALSE;
}

static FrameworkWindow *findEntityNamesEditor(FrameworkWindow *parent) {
  FrameworkWindow *i = parent->children;
  while(i) {
    if(isEntityNamesEditor(i)) {
      return i;
    }
    i = i->next;
  }
  return NULL;
}

void showEntityNamesEditor(FrameworkWindow *parent) {
  FrameworkWindow *entityNamesEditor = findEntityNamesEditor(parent);
  if(entityNamesEditor) {
    WindowToFront(entityNamesEditor->intuitionWindow);
  } else {
    entityNamesEditor = newEntityNamesEditor(parent);
  }
}
