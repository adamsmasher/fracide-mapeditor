#include "EntityNamesEditor.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <proto/gadtools.h>

#include <string.h>

#include "EntityRequester.h"
#include "ProjectWindowData.h"

static FrameworkWindow *entityNamesEditor = NULL;

void closeEntityNamesEditor(void) {
  if(entityNamesEditor) {
    /* TODO: fix me */
    /* removeWindowFromSet(entityNamesEditor->window); */
    /* freeEntityRequester(entityNamesEditor); */
    entityNamesEditor = NULL;
  }
}

void showEntityNamesEditor(FrameworkWindow *parent) {
  if(entityNamesEditor) {
    WindowToFront(entityNamesEditor->intuitionWindow);
  } else {
    entityNamesEditor = newEntityNamesEditor(parent);
  }
}
