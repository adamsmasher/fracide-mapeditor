#include "EntityNamesEditor.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <proto/gadtools.h>

#include <string.h>

#include "EntityRequester.h"
#include "globals.h"
#include "windowset.h"
#include "workspace.h"

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
    int selected = msg->Code;
    int i = listItemStart(selected);

    GT_SetGadgetAttrs(entityNamesEditor->entityNameGadget, entityNamesEditor->window, NULL,
       GTST_String, &project.entityNameStrs[selected][i],
       GA_Disabled, FALSE,
       TAG_END);

    entityNamesEditor->selected = selected + 1;
}

static void handleEntityNamesEditorUpdateEntity(struct IntuiMessage *msg) {
    int selected = entityNamesEditor->selected - 1;
    strcpy(
        &project.entityNameStrs[selected][listItemStart(selected)],
        ((struct StringInfo*)entityNamesEditor->entityNameGadget->SpecialInfo)->Buffer);
    GT_RefreshWindow(entityNamesEditor->window, NULL);
    projectSaved = 0;
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
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(entityNamesEditor->window);
        GT_EndRefresh(entityNamesEditor->window, TRUE);
        break;
    case IDCMP_NEWSIZE:
        resizeEntityRequester(entityNamesEditor);
        break;
    }
}

void handleEntityNamesEditorMessages(long signalSet) {
    struct IntuiMessage *msg;
    if(entityNamesEditor) {
        if(1L << entityNamesEditor->window->UserPort->mp_SigBit & signalSet) {
            while(msg = GT_GetIMsg(entityNamesEditor->window->UserPort)) {
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
        removeWindowFromSet(entityNamesEditor->window);
        freeEntityRequester(entityNamesEditor);
        entityNamesEditor = NULL;
    }
}

void showEntityNamesEditor(void) {
    if(entityNamesEditor) {
        WindowToFront(entityNamesEditor->window);
    } else {
        entityNamesEditor = newEntityNamesEditor();
        if(entityNamesEditor) {
            addWindowToSet(entityNamesEditor->window);
        }
    }
}
