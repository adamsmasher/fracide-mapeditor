#include "SongNames.h"

#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>

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

void showSongNamesEditor(void) {
    if(!songNamesEditor) {
        songNamesEditor = OpenWindow(&songNamesNewWindow);
        if(!songNamesEditor) {
            fprintf(stderr, "showSongNamesEditor: couldn't open window\n");
            return;
        }
    } else {
        WindowToFront(songNamesEditor);
    }
}
