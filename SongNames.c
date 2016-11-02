#include "SongNames.h"

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

#include "globals.h"

#define SONG_NAMES_WIDTH  200
#define SONG_NAMES_HEIGHT 336

#define SONG_LIST_WIDTH        165
#define SONG_LIST_HEIGHT_DELTA 26
#define SONG_LIST_TOP          20
#define SONG_LIST_LEFT         10

#define SONG_NAME_WIDTH         SONG_LIST_WIDTH
#define SONG_NAME_HEIGHT        12
#define SONG_NAME_BOTTOM_OFFSET 26
#define SONG_NAME_LEFT          SONG_LIST_LEFT

static struct NewWindow songNamesNewWindow = {
    40, 40, SONG_NAMES_WIDTH, SONG_NAMES_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Edit Song Names",
    NULL,
    NULL,
    SONG_NAMES_WIDTH, 16,
    SONG_NAMES_WIDTH, 0xFFFF,
    CUSTOMSCREEN
};

static struct NewGadget songListNewGadget = {
    SONG_LIST_LEFT,  SONG_LIST_TOP,
    SONG_LIST_WIDTH, SONG_NAMES_HEIGHT - SONG_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    SONG_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget songNameNewGadget = {
    SONG_NAME_LEFT,  SONG_NAMES_HEIGHT - SONG_NAME_BOTTOM_OFFSET,
    SONG_NAME_WIDTH, SONG_NAME_HEIGHT,
    NULL,
    &Topaz80,
    SONG_NAME_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

void initSongNamesScreen(void) {
    songNamesNewWindow.Screen = screen;
}

void initSongNamesVi(void) {
    songListNewGadget.ng_VisualInfo = vi;
    songNameNewGadget.ng_VisualInfo = vi;
}

static void createSongRequesterGadgets(SongRequester *songRequester) {
    int height;
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    height = songRequester->window ? songRequester->window->Height : SONG_NAMES_HEIGHT;

    if(songRequester->editable) {
        songNameNewGadget.ng_TopEdge = height - SONG_NAME_BOTTOM_OFFSET;
        gad = CreateGadget(STRING_KIND, gad, &songNameNewGadget,
            GTST_MaxChars, 64,
            GA_Disabled, TRUE);
        songRequester->songNameGadget = gad;
    } else {
        songRequester->songNameGadget = NULL;
    }

    songListNewGadget.ng_Height = height - SONG_LIST_HEIGHT_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &songListNewGadget,
        GTLV_ShowSelected, songRequester->songNameGadget,
        GTLV_Labels, &project.songNames,
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
    songNamesNewWindow.Title = songRequester->title;

    createSongRequesterGadgets(songRequester);
    if(!songRequester->gadgets) {
        fprintf(stderr, "newGenericSongRequester: couldn't create gadgets\n");
        goto error_freeTitle;
    }
    songNamesNewWindow.FirstGadget = songRequester->gadgets;

    songRequester->window = OpenWindow(&songNamesNewWindow);
    if(!songRequester->window) {
        fprintf(stderr, "newGenericSongRequester: couldn't open window\n");
        goto error_freeGadgets;
    }
    GT_RefreshWindow(songRequester->window, NULL);

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
    CloseWindow(songRequester->window);
    FreeGadgets(songRequester->gadgets);
    free(songRequester->title);
    free(songRequester);
}

void resizeSongRequester(SongRequester *songRequester) {
    RemoveGList(songRequester->window, songRequester->gadgets, -1);
    FreeGadgets(songRequester->gadgets);
    SetRast(songRequester->window->RPort, 0);
    createSongRequesterGadgets(songRequester);
    if(!songRequester->gadgets) {
        fprintf(stderr, "resizeSongRequester: couldn't make gadgets");
        return;
    }
    AddGList(songRequester->window, songRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(songRequester->window);
    RefreshGList(songRequester->gadgets, songRequester->window, NULL, -1);
    GT_RefreshWindow(songRequester->window, NULL);
}
