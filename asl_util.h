#ifndef ASL_UTIL_H
#define ASL_UTIL_H

#include <stdio.h>

char *aslFilename(const char *dir, const char *file);
FILE *openAsl(const char *dir, const char *file, const char *mode);

#endif
