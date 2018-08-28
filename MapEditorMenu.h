#ifndef MAP_EDITOR_MENU_H
#define MAP_EDITOR_MENU_H

#include "framework/menubuild.h"

extern MenuSpec *mapEditorMenuSpec;

void mapEditorMenuEnableRevertMap(FrameworkWindow*);
void mapEditorMenuDisableRevertMap(FrameworkWindow*);

#endif
