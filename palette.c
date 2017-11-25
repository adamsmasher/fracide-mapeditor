#include "palette.h"

#include <graphics/view.h>
#include <proto/graphics.h>

void initPalette(struct ViewPort *viewport) {
    LONG i;
    ULONG c = 15;
    for(i = 0; i < 4; i++, c -= 5) {
        SetRGB4(viewport, i, c, c, c);
    }
}
