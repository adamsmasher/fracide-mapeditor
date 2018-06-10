#include <proto/exec.h>

#include <proto/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <proto/graphics.h>

#include <libraries/asl.h>
#include <proto/asl.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/runstate.h"
#include "framework/screen.h"

#include "easystructs.h"
#include "EntityBrowser.h"
#include "EntityNamesEditor.h"
#include "EntityRequester.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "palette.h"
#include "ProjectWindow.h"
#include "SongNamesEditor.h"
#include "SongRequester.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

#define SCR_WIDTH  640
#define SCR_HEIGHT 512

static struct NewScreen newScreen = {
  0,0,SCR_WIDTH,SCR_HEIGHT,2,
  0,3,
  HIRES|LACE,
  CUSTOMSCREEN,
  NULL,
  "FracIDE Map Editor",
  NULL,
  NULL
};

int main(void) {
  int errorCode;
  FrameworkWindow *projectWindow;

  if(!initGlobalScreen(&newScreen)) {
    errorCode = -1;
    goto error;
  }

  initPalette(getGlobalViewPort());

  projectWindow = openProjectWindow();
  if(!projectWindow) {
    errorCode = -2;
    goto error_closeScreen;
  }
    
  runMainLoop(projectWindow);

closeScreen:
  closeGlobalScreen();
done:
  return 0;

error_closeScreen:
  closeGlobalScreen();
error:
  return errorCode;
}
