#include "windowset.h"

static long sigMask = 0;

void addWindowToSet(struct Window *window) {
    sigMask |= 1L << window->UserPort->mp_SigBit;
}

void removeWindowFromSet(struct Window *window) {
    sigMask &= ~(1L << window->UserPort->mp_SigBit);
}

long windowSetSigMask(void) {
    return sigMask;
}
