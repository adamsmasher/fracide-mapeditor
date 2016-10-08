#ifndef SONG_NAMES_H
#define SONG_NAMES_H

#include <proto/intuition.h>

typedef struct SongNamesEditor_tag {
    struct Window *window;
    int closed;
} SongNamesEditor;

void initSongNamesScreen(void);

SongNamesEditor *newSongNamesEditor(void);
void freeSongNamesEditor(SongNamesEditor*);

#endif
