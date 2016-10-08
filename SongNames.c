#include "SongNames.h"

#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

#define SONG_NAMES_WIDTH  200
#define SONG_NAMES_HEIGHT 336

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

void initSongNamesScreen(void) {
    songNamesNewWindow.Screen = screen;
}

SongNamesEditor *newSongNamesEditor(void) {
    SongNamesEditor *songNamesEditor = malloc(sizeof(SongNamesEditor));
    if(!songNamesEditor) {
        fprintf(stderr, "newSongNamesEditor: couldn't allocate editor\n");
        goto error;
    }

    songNamesEditor->window = OpenWindow(&songNamesNewWindow);
    if(!songNamesEditor->window) {
        fprintf(stderr, "showSongNamesEditor: couldn't open window\n");
        goto error_freeEditor;
    }

    songNamesEditor->closed = 0;

    return songNamesEditor;

error_freeEditor:
    free(songNamesEditor);
error:
    return NULL;
}

void freeSongNamesEditor(SongNamesEditor *songNamesEditor) {
    CloseWindow(songNamesEditor->window);
    free(songNamesEditor);
}

