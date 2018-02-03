#include "menu.h"

#include "menubuild.h"
#include "window.h"

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
  MenuSpec *menuSpec = window->kind->menuSpec;
  UWORD menuNum = MENUNUM(menuNumber);
  UWORD itemNum = ITEMNUM(menuNumber);

  Handler handler = getMenuHandler(menuSpec, menuNum, itemNum);
  handler(window);
}
