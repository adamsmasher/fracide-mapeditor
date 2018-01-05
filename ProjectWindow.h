#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

struct Window *getProjectWindow(void);

BOOL openProjectWindow(struct Screen*);
void closeProjectWindow(void);

void handleProjectMessages(long signalSet);

#endif
