#ifndef FRAC_WINDOWSET_H
#define FRAC_WINDOWSET_H

#include <intuition/intuition.h>

void addWindowToSet(struct Window*);
void removeWindowFromSet(struct Window*);

long windowSetSigMask(void);

#endif