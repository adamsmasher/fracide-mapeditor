#ifndef SONG_NAMES_H
#define SONG_NAMES_H

#include <proto/intuition.h>

#define SONG_LIST_ID (0)
#define SONG_NAME_ID (SONG_LIST_ID + 1)

typedef struct SongNamesEditor_tag {
    struct Window *window;
    struct Gadget *gadgets;
    int closed;
} SongNamesEditor;

void initSongNamesScreen(void);
void initSongNamesVi(void);

SongNamesEditor *newSongNamesEditor(void);
void freeSongNamesEditor(SongNamesEditor*);

#endif
