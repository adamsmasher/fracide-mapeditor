#ifndef RUNSTATE_H
#define RUNSTATE_H

#include <exec/types.h>

typedef void (*RunProc)(long signalSet);

void run(RunProc);

void stopRunning(void);

#endif
