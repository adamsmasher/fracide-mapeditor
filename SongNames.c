#include "SongNames.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
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

#define SONG_NAME_WIDTH  SONG_LIST_WIDTH
#define SONG_NAME_HEIGHT 12
#define SONG_NAME_TOP    310
#define SONG_NAME_LEFT   SONG_LIST_LEFT

/* TODO: make this resizeable up and down */
static struct NewWindow songNamesNewWindow = {
    40, 40, SONG_NAMES_WIDTH, SONG_NAMES_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP,
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

static struct NewGadget songNameNewGadget = {
    SONG_NAME_LEFT,  SONG_NAME_TOP,
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

static void createSongRequesterGadgets(SongRequester *songRequester, int editable) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    if(editable) {
        gad = CreateGadget(STRING_KIND, gad, &songNameNewGadget,
            GTST_MaxChars, 64,
            GA_Disabled, TRUE);
        songRequester->songNameGadget = gad;
    } else {
        songRequester->songNameGadget = NULL;
    }

    gad = CreateGadget(LISTVIEW_KIND, gad, &songListNewGadget,
        GTLV_ShowSelected, songRequester->songNameGadget,
        GTLV_Labels, &project.songNames,
        TAG_END);

    if(gad) {
        songRequester->gadgets = glist;
    } else {
        FreeGadgets(glist);
        songRequester->songNameGadget = NULL;
    }
}

static SongRequester *newGenericSongRequester(int editable) {
    SongRequester *songRequester = malloc(sizeof(SongRequester));
    if(!songRequester) {
        fprintf(stderr, "newGenericSongRequester: couldn't allocate requester\n");
        goto error;
    }

    createSongRequesterGadgets(songRequester, editable);
    if(!songRequester->gadgets) {
        fprintf(stderr, "newGenericSongRequester: couldn't create gadgets\n");
        goto error_freeRequester;
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
error_freeRequester:
    free(songRequester);
error:
    return NULL;
}

#define EDITABLE 1
#define NON_EDITABLE 0

SongRequester *newSongRequester(char *title) {
    songNamesNewWindow.Title = title;
    return newGenericSongRequester(NON_EDITABLE);
}

SongRequester *newSongNamesEditor(void) {
    songNamesNewWindow.Title = "Edit Song Names";
    return newGenericSongRequester(EDITABLE);
}

void freeSongRequester(SongRequester *songRequester) {
    CloseWindow(songRequester->window);
    FreeGadgets(songRequester->gadgets);
    free(songRequester);
}
