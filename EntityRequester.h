#ifndef ENTITY_REQUESTER_H
#define ENTITY_REQUESTER_H

#include "framework/Window.h"

FrameworkWindow *newEntityRequester(FrameworkWindow *entityBrowser);
FrameworkWindow *newEntityNamesEditor(FrameworkWindow *projectWindow);

BOOL isEntityNamesEditor(FrameworkWindow *window);

#endif
