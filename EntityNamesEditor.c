#include "EntityNamesEditor.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <proto/gadtools.h>

#include <string.h>

#include "EntityRequester.h"
#include "ProjectWindowData.h"
#include "workspace.h"

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

static EntityRequester *entityNamesEditor = NULL;

static void handleEntityNamesEditorSelectEntity(struct IntuiMessage *msg) {
  ProjectWindowData *projectData = NULL; /* TODO fix me */
  int selected = msg->Code;

  GT_SetGadgetAttrs(entityNamesEditor->entityNameGadget, entityNamesEditor->window->intuitionWindow, NULL,
    GTST_String, projectDataGetEntityName(projectData, selected),
    GA_Disabled, FALSE,
    TAG_END);

  entityNamesEditor->selected = selected + 1;
}

static void handleEntityNamesEditorUpdateEntity(struct IntuiMessage *msg) {
  ProjectWindowData *projectData = NULL; /* TODO: fix me */
  int selected = entityNamesEditor->selected - 1;
  char *name = ((struct StringInfo*)entityNamesEditor->entityNameGadget->SpecialInfo)->Buffer;

  projectDataUpdateEntityName(projectData, selected, name);

  GT_RefreshWindow(entityNamesEditor->window->intuitionWindow, NULL);
  refreshAllEntityBrowsers();
}

static void handleEntityNamesEditorGadgetUp(struct IntuiMessage *msg) {
    struct Gadget *gadget = (struct Gadget*)msg->IAddress;
    switch(gadget->GadgetID) {
    case ENTITY_REQUESTER_LIST_ID:
        handleEntityNamesEditorSelectEntity(msg);
        break;
    case ENTITY_NAME_ID:
        handleEntityNamesEditorUpdateEntity(msg);
        break;
    }
}

static void handleEntityNamesEditorMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        entityNamesEditor->closed = 1;
        break;
    case IDCMP_GADGETUP:
        handleEntityNamesEditorGadgetUp(msg);
        break;
    case IDCMP_NEWSIZE:
        resizeEntityRequester(entityNamesEditor);
        break;
    }
}

void handleEntityNamesEditorMessages(long signalSet) {
    struct IntuiMessage *msg;
    if(entityNamesEditor) {
        if(1L << entityNamesEditor->window->intuitionWindow->UserPort->mp_SigBit & signalSet) {
            while(msg = GT_GetIMsg(entityNamesEditor->window->intuitionWindow->UserPort)) {
                handleEntityNamesEditorMessage(msg);
                GT_ReplyIMsg(msg);
            }
        }
        if(entityNamesEditor->closed) {
            closeEntityNamesEditor();
        }
    }
}


void closeEntityNamesEditor(void) {
    if(entityNamesEditor) {
        /* TODO: fix me */
        /* removeWindowFromSet(entityNamesEditor->window); */
        freeEntityRequester(entityNamesEditor);
        entityNamesEditor = NULL;
    }
}

void showEntityNamesEditor(void) {
    if(entityNamesEditor) {
        WindowToFront(entityNamesEditor->window->intuitionWindow);
    } else {
        entityNamesEditor = newEntityNamesEditor();
        if(entityNamesEditor) {
            /* TODO: fix me */
            /* addWindowToSet(entityNamesEditor->window); */
        }
    }
}
