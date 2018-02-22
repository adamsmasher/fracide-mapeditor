#include "framework/menu.h"

#include "menubuild.h"
#include "window.h"

#include <stdio.h>

Handler getMenuItemHandler(MenuSpec *menuSpec, UWORD itemNum) {
  MenuSectionSpec **sectionSpec = &(*menuSpec->sections)[0];
  MenuItemSpec *itemSpec = &(**sectionSpec)[0];

  while(itemNum) {
    itemSpec++;
    if(endMenuSection(itemSpec)) {
      sectionSpec++;
      itemSpec = &(**sectionSpec)[0];
      /* skip past the end of section marker */
      itemNum--;
    }

    itemNum--;
  }

  return itemSpec->handler;
}

Handler getMenuHandler(MenuSpec *menusSpec, UWORD menuNum, UWORD itemNum) {
  MenuSpec *menuSpec = &menusSpec[menuNum];
  return getMenuItemHandler(menuSpec, itemNum);
}

void invokeMenuHandler(FrameworkWindow *window, ULONG menuNumber) {
  Handler handler;
  MenuSpec *menuSpec = window->kind->menuSpec;
  UWORD menuNum = MENUNUM(menuNumber);
  UWORD itemNum = ITEMNUM(menuNumber);

  if(!menuSpec) {
    fprintf(stderr, "invokeMenuHandler: attempted to invoke a menu handler on a window with no menu\n");
    goto error;
  }

  handler = getMenuHandler(menuSpec, menuNum, itemNum);

  handler(window);
done:
  return;

error:
  return;
}
