#ifndef SONG_REQUESTER_H
#define SONG_REQUESTER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#define SONG_LIST_ID (0)
#define SONG_NAME_ID (SONG_LIST_ID + 1)

typedef struct SongRequester_tag {
    FrameworkWindow *window;
    struct Gadget *gadgets;
    struct Gadget *songNameGadget;
    char *title;
    int closed;
    int selected;
    int editable;
} SongRequester;

SongRequester *newSongRequester(char *title);
SongRequester *newSongNamesEditor(void);

void resizeSongRequester(SongRequester*);

void freeSongRequester(SongRequester*);

#endif
