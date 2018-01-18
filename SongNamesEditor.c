#include "SongNamesEditor.h"

#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <string.h>

#include "framework/windowset.h"

#include "currentproject.h"
#include "globals.h"
#include "SongRequester.h"
#include "workspace.h"

SongRequester *songNamesEditor = NULL;

static void songNamesEditorSelectSong(int songNum) {
  GT_SetGadgetAttrs(songNamesEditor->songNameGadget, songNamesEditor->window, NULL,
    GTST_String, currentProjectGetSongName(songNum),
    GA_Disabled, FALSE,
    TAG_END);

  songNamesEditor->selected = songNum + 1;
}

static void songNamesEditorUpdateSelectedSong(void) {
  int selected = songNamesEditor->selected - 1;

  char *name = ((struct StringInfo*)songNamesEditor->songNameGadget->SpecialInfo)->Buffer;
  updateCurrentProjectSongName(selected, name);

  GT_RefreshWindow(songNamesEditor->window, NULL);

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
  case IDCMP_REFRESHWINDOW:
    GT_BeginRefresh(songNamesEditor->window);
    GT_EndRefresh(songNamesEditor->window, TRUE);
    break;
  case IDCMP_NEWSIZE:
    resizeSongRequester(songNamesEditor);
    break;
  }
}

void closeSongNamesEditor(void) {
  if(songNamesEditor) {
    removeWindowFromSet(songNamesEditor->window);
    freeSongRequester(songNamesEditor);
    songNamesEditor = NULL;
  }
}

void handleSongNamesEditorMessages(long signalSet) {
  struct IntuiMessage *msg;
  if(songNamesEditor) {
    if(1L << songNamesEditor->window->UserPort->mp_SigBit & signalSet) {
      while(msg = GT_GetIMsg(songNamesEditor->window->UserPort)) {
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
    WindowToFront(songNamesEditor->window);
  } else {
    songNamesEditor = newSongNamesEditor();
    if(songNamesEditor) {
      addWindowToSet(songNamesEditor->window);
    }
  }
}
