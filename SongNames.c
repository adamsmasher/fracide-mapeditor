#include "SongNames.h"

#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

#define SONG_NAMES_WIDTH  200
#define SONG_NAMES_HEIGHT 336

#define SONG_LIST_WIDTH  165
#define SONG_LIST_HEIGHT 310
#define SONG_LIST_TOP    20
#define SONG_LIST_LEFT   10

/* TODO: make this resizeable up and down */
static struct NewWindow songNamesNewWindow = {
    40, 40, SONG_NAMES_WIDTH, SONG_NAMES_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Edit Song Names",
    NULL,
    NULL,
    SONG_NAMES_WIDTH, SONG_NAMES_HEIGHT,
    SONG_NAMES_WIDTH, SONG_NAMES_HEIGHT,
    CUSTOMSCREEN
};

static struct NewGadget songListNewGadget = {
    SONG_LIST_LEFT,  SONG_LIST_TOP,
    SONG_LIST_WIDTH, SONG_LIST_HEIGHT,
    NULL,
    &Topaz80,
    SONG_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

void initSongNamesScreen(void) {
    songNamesNewWindow.Screen = screen;
}

void initSongNamesVi(void) {
    songListNewGadget.ng_VisualInfo = vi;
}

static struct Gadget *createSongNamesGadgets(void) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    /* TODO: use GTLV_Labels to get song names;
             use GTLV_ShowSelected and a STRING gadget to edit the names */
    gad = CreateGadget(LISTVIEW_KIND, gad, &songListNewGadget,
        TAG_END);

    if(gad) {
        return glist;
    } else {
        FreeGadgets(glist);
        return NULL;
    }
}

SongNamesEditor *newSongNamesEditor(void) {
    SongNamesEditor *songNamesEditor = malloc(sizeof(SongNamesEditor));
    if(!songNamesEditor) {
        fprintf(stderr, "newSongNamesEditor: couldn't allocate editor\n");
        goto error;
    }

    songNamesEditor->gadgets = createSongNamesGadgets();
    if(!songNamesEditor->gadgets) {
        fprintf(stderr, "newSongNamesEditor: couldn't create gadgets\n");
        goto error_freeEditor;
    }
    songNamesNewWindow.FirstGadget = songNamesEditor->gadgets;

    songNamesEditor->window = OpenWindow(&songNamesNewWindow);
    if(!songNamesEditor->window) {
        fprintf(stderr, "showSongNamesEditor: couldn't open window\n");
        goto error_freeGadgets;
    }
    GT_RefreshWindow(songNamesEditor->window, NULL);

    songNamesEditor->closed = 0;

    return songNamesEditor;

error_freeGadgets:
    free(songNamesEditor->gadgets);
error_freeEditor:
    free(songNamesEditor);
error:
    return NULL;
}

void freeSongNamesEditor(SongNamesEditor *songNamesEditor) {
    CloseWindow(songNamesEditor->window);
    FreeGadgets(songNamesEditor->gadgets);
    free(songNamesEditor);
}

