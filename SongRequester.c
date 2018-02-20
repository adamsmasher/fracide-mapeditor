#include "SongRequester.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/screen.h"

#include "currentproject.h"
#include "globals.h"

#define SONG_REQUESTER_WIDTH      200
#define SONG_REQUESTER_HEIGHT     336
#define SONG_REQUESTER_MIN_HEIGHT 48

#define SONG_LIST_WIDTH_DELTA  35
#define SONG_LIST_HEIGHT_DELTA 26
#define SONG_LIST_TOP          20
#define SONG_LIST_LEFT         10

#define SONG_NAME_WIDTH_DELTA   SONG_LIST_WIDTH_DELTA
#define SONG_NAME_HEIGHT        12
#define SONG_NAME_BOTTOM_OFFSET 26
#define SONG_NAME_LEFT          SONG_LIST_LEFT

static WindowKind songRequesterWindowKind = {
  {
    40, 40, SONG_REQUESTER_WIDTH, SONG_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Edit Song Names",
    NULL,
    NULL,
    SONG_REQUESTER_WIDTH, SONG_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  NULL
};

static struct NewGadget songListNewGadget = {
    SONG_LIST_LEFT,  SONG_LIST_TOP,
    SONG_REQUESTER_WIDTH - SONG_LIST_WIDTH_DELTA,
    SONG_REQUESTER_HEIGHT - SONG_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    SONG_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget songNameNewGadget = {
    SONG_NAME_LEFT,  SONG_REQUESTER_HEIGHT - SONG_NAME_BOTTOM_OFFSET,
    SONG_REQUESTER_WIDTH - SONG_NAME_WIDTH_DELTA, SONG_NAME_HEIGHT,
    NULL,
    &Topaz80,
    SONG_NAME_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static void initSongRequesterVi(void) {
  void *vi = getGlobalVi();
  if(!vi) {
    fprintf(stderr, "initSongRequesterVi: failed to get global vi\n");
  }

  songListNewGadget.ng_VisualInfo = vi;
  songNameNewGadget.ng_VisualInfo = vi;
}

static void createSongRequesterGadgets(SongRequester *songRequester) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;
    int height = songRequester->window ? songRequester->window->intuitionWindow->Height : SONG_REQUESTER_HEIGHT;
    int width  = songRequester->window ? songRequester->window->intuitionWindow->Width  : SONG_REQUESTER_WIDTH;

    gad = CreateContext(&glist);

    if(songRequester->editable) {
        songNameNewGadget.ng_TopEdge = height - SONG_NAME_BOTTOM_OFFSET;
        songNameNewGadget.ng_Width   = width  - SONG_NAME_WIDTH_DELTA;
        gad = CreateGadget(STRING_KIND, gad, &songNameNewGadget,
            GTST_MaxChars, 64,
            GA_Disabled, TRUE);
        songRequester->songNameGadget = gad;
    } else {
        songRequester->songNameGadget = NULL;
    }

    songListNewGadget.ng_Height = height - SONG_LIST_HEIGHT_DELTA;
    songListNewGadget.ng_Width  = width  - SONG_LIST_WIDTH_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &songListNewGadget,
        GTLV_ShowSelected, songRequester->songNameGadget,
        GTLV_Labels, currentProjectGetSongNames(),
        TAG_END);

    if(gad) {
        songRequester->gadgets = glist;
    } else {
        songRequester->songNameGadget = NULL;
        FreeGadgets(glist);
    }
}

static SongRequester *newGenericSongRequester(char *title, int editable) {
    SongRequester *songRequester = malloc(sizeof(SongRequester));
    if(!songRequester) {
        fprintf(stderr, "newGenericSongRequester: couldn't allocate requester\n");
        goto error;
    }
    songRequester->window = NULL;
    songRequester->editable = editable;

    songRequester->title = malloc(strlen(title) + 1);
    if(!songRequester->title) {
        fprintf(stderr, "newGenericSongRequester: couldn't allocate title\n");
        goto error_freeRequester;
    }
    strcpy(songRequester->title, title);
    songRequesterWindowKind.newWindow.Title = songRequester->title;

    initSongRequesterVi();
    createSongRequesterGadgets(songRequester);
    if(!songRequester->gadgets) {
        fprintf(stderr, "newGenericSongRequester: couldn't create gadgets\n");
        goto error_freeTitle;
    }
    songRequesterWindowKind.newWindow.FirstGadget = songRequester->gadgets;

    songRequester->window = openWindowOnGlobalScreen(&songRequesterWindowKind);
    if(!songRequester->window) {
        fprintf(stderr, "newGenericSongRequester: couldn't open window\n");
        goto error_freeGadgets;
    }
    GT_RefreshWindow(songRequester->window->intuitionWindow, NULL);

    songRequester->closed = 0;
    songRequester->selected = 0;

    return songRequester;

error_freeGadgets:
    free(songRequester->gadgets);
error_freeTitle:
    free(songRequester->title);
error_freeRequester:
    free(songRequester);
error:
    return NULL;
}

#define EDITABLE 1
#define NON_EDITABLE 0

SongRequester *newSongRequester(char *title) {
    return newGenericSongRequester(title, NON_EDITABLE);
}

SongRequester *newSongNamesEditor(void) {
    return newGenericSongRequester("Edit Song Names", EDITABLE);
}

void freeSongRequester(SongRequester *songRequester) {
  /* TODO: the framework should close the window and free the gadgets */
  free(songRequester->title);
  free(songRequester);
}

void resizeSongRequester(SongRequester *songRequester) {
    /* TODO: this should be done by the framework! */
    RemoveGList(songRequester->window->intuitionWindow, songRequester->gadgets, -1);
    FreeGadgets(songRequester->gadgets);
    SetRast(songRequester->window->intuitionWindow->RPort, 0);
    createSongRequesterGadgets(songRequester);
    if(!songRequester->gadgets) {
        fprintf(stderr, "resizeSongRequester: couldn't make gadgets");
        return;
    }
    AddGList(songRequester->window->intuitionWindow, songRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(songRequester->window->intuitionWindow);
    RefreshGList(songRequester->gadgets, songRequester->window->intuitionWindow, NULL, -1);
    GT_RefreshWindow(songRequester->window->intuitionWindow, NULL);
}
