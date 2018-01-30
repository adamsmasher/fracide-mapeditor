#include "menu.h"

#include "menubuild.h"

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

/* TODO: not sure about these arguments - maybe have the window store the menu store the spec? */
void invokeMenuHandler(FrameworkWindow *window, MenuSpec *menusSpec, ULONG menuNumber) {
  UWORD menuNum = MENUNUM(menuNumber);
  UWORD itemNum = ITEMNUM(menuNumber);

  Handler handler = getMenuHandler(menusSpec, menuNum, itemNum);
  handler(window);
}
