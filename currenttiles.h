#ifndef FRAC_CURRENTTILES_H
#define FRAC_CURRENTTILES_H

#include "TilesetPackage.h"

/* TODO: don't expose me */
extern TilesetPackage  *tilesetPackage;

int loadTilesetPackageFromAsl(char *dir, char *file);
int loadTilesetPackageFromFile(char*);

#endif
