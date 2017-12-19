#ifndef FRAC_CURRENT_PROJECT_H
#define FRAC_CURRENT_PROJECT_H

#include <exec/types.h>

BOOL ensureProjectSaved(void);

void clearProject(void);

void setProjectFilename(char*);

void openProjectFromAsl(char *dir, char *file);
void openProjectFromFile(char*);

int saveProject(void);
int saveProjectAs(void);

#endif
