#ifndef SONG_NAMES_H
#define SONG_NAMES_H

#include <proto/intuition.h>

#define SONG_LIST_ID (0)
#define SONG_NAME_ID (SONG_LIST_ID + 1)

typedef struct SongRequester_tag {
    struct Window *window;
    struct Gadget *gadgets;
    struct Gadget *songNameGadget;
    char *title;
    int closed;
    int selected;
    int editable;
} SongRequester;

void initSongNamesScreen(void);
void initSongNamesVi(void);

SongRequester *newSongRequester(char *title);
SongRequester *newSongNamesEditor(void);

void freeSongRequester(SongRequester*);

#endif
