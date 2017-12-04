#ifndef FRAC_CURRENT_PROJECT_H
#define FRAC_CURRENT_PROJECT_H

BOOL ensureProjectSaved(void);

void clearProject(void);

void setProjectFilename(char*);

void openProjectFromAsl(char *dir, char *file);
void openProjectFromFile(char*);

int saveProjectToAsl(char *dir, char *file);

#endif
