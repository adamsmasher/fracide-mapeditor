#ifndef PROJECT_WINDOW_MENU_H
#define PROJECT_WINDOW_MENU_H

#include "framework/menubuild.h"

extern MenuSpec *projectWindowMenuSpec;

void projectWindowMenuEnableRevertProject(FrameworkWindow*);
void projectWindowMenuDisableRevertProject(FrameworkWindow*);

#endif
