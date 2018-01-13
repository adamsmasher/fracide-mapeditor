#ifndef FRAC_CURRENT_PROJECT_H
#define FRAC_CURRENT_PROJECT_H

#include <exec/types.h>
#include "Project.h"

BOOL ensureProjectSaved(void);

void clearProject(void);

void setProjectFilename(char*);
char *getProjectFilename(void);

void openProjectFromAsl(char *dir, char *file);
void openProjectFromFile(char*);

int saveProject(void);
int saveProjectAs(void);

/* TODO: don't expose us */
extern Project project;
extern int     projectSaved;

#endif
