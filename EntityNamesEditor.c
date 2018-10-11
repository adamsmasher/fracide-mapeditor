#include "EntityNamesEditor.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <proto/gadtools.h>

#include <string.h>

#include "EntityRequester.h"
#include "ProjectWindowData.h"

/* TODO help i'm duplicated everywhere */
static int listItemStart(int selected) {
  if(selected < 10) {
    return 2;
  } else if(selected < 100) {
    return 3;
  } else {
    return 4;
  }
}

static FrameworkWindow *entityNamesEditor = NULL;

static void handleEntityNamesEditorSelectEntity(struct IntuiMessage *msg) {
  EntityRequesterData *data = entityNamesEditor->data;
  EntityRequesterGadgets *gadgets = entityNamesEditor->gadgets->data;
  ProjectWindowData *projectData = NULL; /* TODO fix me */
  int selected = msg->Code;

  GT_SetGadgetAttrs(gadgets->entityNameGadget, entityNamesEditor->intuitionWindow, NULL,
    GTST_String, projectDataGetEntityName(projectData, selected),
    GA_Disabled, FALSE,
    TAG_END);

  data->selected = selected + 1;
}

static void handleEntityNamesEditorUpdateEntity(struct IntuiMessage *msg) {
  EntityRequesterData *data = entityNamesEditor->data;
  EntityRequesterGadgets *gadgets = entityNamesEditor->gadgets->data;
  ProjectWindowData *projectData = NULL; /* TODO: fix me */
  int selected = data->selected - 1;
  char *name = ((struct StringInfo*)gadgets->entityNameGadget->SpecialInfo)->Buffer;

  projectDataUpdateEntityName(projectData, selected, name);

  GT_RefreshWindow(entityNamesEditor->intuitionWindow, NULL);
  refreshAllEntityBrowsers();
}

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
