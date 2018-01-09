#ifndef FRAC_WINDOWSET_H
#define FRAC_WINDOWSET_H

/* TODO: put me in the framework */

#include <intuition/intuition.h>

void addWindowToSet(struct Window*);
void removeWindowFromSet(struct Window*);

long windowSetSigMask(void);

#endif