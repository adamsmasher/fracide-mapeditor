#include "SongNamesEditor.h"

#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <string.h>

#include "framework/windowset.h"

#include "SongRequester.h"
#include "ProjectWindowData.h"
#include "workspace.h"

SongRequester *songNamesEditor = NULL;

static void songNamesEditorSelectSong(int songNum) {
  ProjectWindowData *projectData = NULL; /* TODO: fix me */

  GT_SetGadgetAttrs(songNamesEditor->songNameGadget, songNamesEditor->window->intuitionWindow, NULL,
    GTST_String, projectDataGetSongName(projectData, songNum),
    GA_Disabled, FALSE,
    TAG_END);

  songNamesEditor->selected = songNum + 1;
}

static void songNamesEditorUpdateSelectedSong(void) {
  ProjectWindowData *projectData = NULL; /* TODO: fix me */

  int selected = songNamesEditor->selected - 1;

  char *name = ((struct StringInfo*)songNamesEditor->songNameGadget->SpecialInfo)->Buffer;
  projectDataUpdateSongName(projectData, selected, name);

  GT_RefreshWindow(songNamesEditor->window->intuitionWindow, NULL);

  refreshAllSongDisplays();
}

static void handleSongNamesEditorGadgetUp(struct IntuiMessage *msg) {
  struct Gadget *gadget = (struct Gadget*)msg->IAddress;
  switch(gadget->GadgetID) {
  case SONG_LIST_ID:
    songNamesEditorSelectSong(msg->Code);
    break;
  case SONG_NAME_ID:
    songNamesEditorUpdateSelectedSong();
    break;
  }
}

static void handleSongNamesEditorMessage(struct IntuiMessage* msg) {
  switch(msg->Class) {
  case IDCMP_CLOSEWINDOW:
    songNamesEditor->closed = 1;
    break;
  case IDCMP_GADGETUP:
    handleSongNamesEditorGadgetUp(msg);
    break;
  case IDCMP_NEWSIZE:
    resizeSongRequester(songNamesEditor);
    break;
  }
}

void closeSongNamesEditor(void) {
  if(songNamesEditor) {
    /* TODO: fix me */
    /* removeWindowFromSet(songNamesEditor->window); */
    freeSongRequester(songNamesEditor);
    songNamesEditor = NULL;
  }
}

void handleSongNamesEditorMessages(long signalSet) {
  struct IntuiMessage *msg;
  if(songNamesEditor) {
    if(1L << songNamesEditor->window->intuitionWindow->UserPort->mp_SigBit & signalSet) {
      while(msg = GT_GetIMsg(songNamesEditor->window->intuitionWindow->UserPort)) {
        handleSongNamesEditorMessage(msg);
        GT_ReplyIMsg(msg);
      }
      if(songNamesEditor->closed) {
        closeSongNamesEditor();
      }
    }
  }
}

void showSongNamesEditor(void) {
  if(songNamesEditor) {
    WindowToFront(songNamesEditor->window->intuitionWindow);
  } else {
    songNamesEditor = newSongNamesEditor();
    if(songNamesEditor) {
      /* TODO: fix me */
      /* addWindowToSet(songNamesEditor->window); */
    }
  }
}
