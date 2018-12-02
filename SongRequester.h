#ifndef SONG_REQUESTER_H
#define SONG_REQUESTER_H

#include "framework/Window.h"

FrameworkWindow *newSongRequester(FrameworkWindow *mapEditor);
FrameworkWindow *newSongNamesEditor(FrameworkWindow *projectWindow);

BOOL isSongRequester(FrameworkWindow*);
BOOL isSongNamesEditor(FrameworkWindow*);

void songRequesterRefresh(FrameworkWindow *songRequester);

#endif
