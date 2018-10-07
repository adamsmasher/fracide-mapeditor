#include "Gadgets.h"

#include <proto/gadtools.h>
#include <intuition/gadgetclass.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "font.h"
#include "screen.h"

union GadgetSpecBody {
  ButtonSpec   *buttonSpec;
  ScrollerSpec *scrollerSpec;
  StringSpec   *stringSpec;
  TextSpec     *textSpec;
  IntegerSpec  *integerSpec;
  ListViewSpec *listViewSpec;
};

struct GadgetSpec_tag {
  unsigned long kind;
  union GadgetSpecBody specBody;
};

GadgetSpec *makeButtonGadget(ButtonSpec *buttonSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = BUTTON_KIND;
  gadgetSpec->specBody.buttonSpec = buttonSpec;
  return gadgetSpec;
}

GadgetSpec *makeScrollerGadget(ScrollerSpec *scrollerSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = SCROLLER_KIND;
  gadgetSpec->specBody.scrollerSpec = scrollerSpec;
  return gadgetSpec;
}

GadgetSpec *makeStringGadget(StringSpec *stringSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = STRING_KIND;
  gadgetSpec->specBody.stringSpec = stringSpec;
  return gadgetSpec;
}

GadgetSpec *makeTextGadget(TextSpec *textSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = TEXT_KIND;
  gadgetSpec->specBody.textSpec = textSpec;
  return gadgetSpec;
}

GadgetSpec *makeIntegerGadget(IntegerSpec *integerSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = INTEGER_KIND;
  gadgetSpec->specBody.integerSpec = integerSpec;
  return gadgetSpec;
}

GadgetSpec *makeListViewGadget(ListViewSpec *listViewSpec) {
  GadgetSpec *gadgetSpec = malloc(sizeof(GadgetSpec));
  gadgetSpec->kind = LISTVIEW_KIND;
  gadgetSpec->specBody.listViewSpec = listViewSpec;
  return gadgetSpec;
}

static BOOL borderToBorderTag(Border border) {
  switch(border) {
    case NO_BORDER: return FALSE;
    case BORDERED: return TRUE;
    default:
      fprintf(stderr, "borderToBorderTag: invalid border %d\n", border);
      return FALSE;
  }
}

static BOOL stateToDisabledTag(State state) {
  switch(state) {
    case ENABLED:  return FALSE;
    case DISABLED: return TRUE;
    default:
      fprintf(stderr, "stateToDisabledTag: invalid state %d\n", state);
      return FALSE;
  }
}

static int orientationToFreedomTag(Orientation orientation) {
  switch(orientation) {
    case HORIZONTAL: return LORIENT_HORIZ;
    case VERTICAL: return LORIENT_VERT;
    default:
      fprintf(stderr, "orientationToFreedomTag: invalid orientation %d\n", orientation);
      return LORIENT_NONE;
  }
}

static ULONG textPlacementToFlags(TextPlacement textPlacement) {
  switch(textPlacement) {
    case TEXT_ON_LEFT:  return PLACETEXT_LEFT;
    case TEXT_ON_RIGHT: return PLACETEXT_RIGHT;
    case TEXT_ABOVE:    return PLACETEXT_ABOVE;
    case TEXT_BELOW:    return PLACETEXT_BELOW;
    case TEXT_INSIDE:   return PLACETEXT_IN;
    default:
      fprintf(stderr, "textPlacementToFlags: invalid text placement %d\n", textPlacement);
      return -1;
  }
}

static struct Gadget *buildButton(ButtonSpec *buttonSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = buttonSpec->left;
  newGadget.ng_TopEdge    = buttonSpec->top;
  newGadget.ng_Width      = buttonSpec->width;
  newGadget.ng_Height     = buttonSpec->height;
  newGadget.ng_GadgetText = buttonSpec->label;
  newGadget.ng_TextAttr   = &Topaz80;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = textPlacementToFlags(buttonSpec->textPlacement);
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = (void*)buttonSpec->onClick;
  return CreateGadget(BUTTON_KIND, context, &newGadget,
    GA_Disabled, stateToDisabledTag(buttonSpec->state),
    TAG_END);
}

static struct Gadget *buildScroller(ScrollerSpec *scrollerSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = scrollerSpec->left;
  newGadget.ng_TopEdge    = scrollerSpec->top;
  newGadget.ng_Width      = scrollerSpec->width;
  newGadget.ng_Height     = scrollerSpec->height;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_TextAttr   = NULL;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = 0;
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = NULL;
  return CreateGadget(SCROLLER_KIND, context, &newGadget,
    GA_Disabled, stateToDisabledTag(scrollerSpec->state),
    PGA_Freedom, orientationToFreedomTag(scrollerSpec->orientation),
    TAG_END);
}

static struct Gadget *buildString(StringSpec *stringSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = stringSpec->left;
  newGadget.ng_TopEdge    = stringSpec->top;
  newGadget.ng_Width      = stringSpec->width;
  newGadget.ng_Height     = stringSpec->height;
  newGadget.ng_GadgetText = stringSpec->label;
  newGadget.ng_TextAttr   = &Topaz80;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = textPlacementToFlags(stringSpec->textPlacement);
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = (void*)stringSpec->onEntry;
  return CreateGadget(STRING_KIND, context, &newGadget,
    GA_Disabled, stateToDisabledTag(stringSpec->state),
    TAG_END);
}

static struct Gadget *buildText(TextSpec *textSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = textSpec->left;
  newGadget.ng_TopEdge    = textSpec->top;
  newGadget.ng_Width      = textSpec->width;
  newGadget.ng_Height     = textSpec->height;
  newGadget.ng_GadgetText = textSpec->label;
  newGadget.ng_TextAttr   = &Topaz80;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = textPlacementToFlags(textSpec->textPlacement);
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = NULL;
  return CreateGadget(TEXT_KIND, context, &newGadget,
    GTTX_Text, textSpec->text,
    GTTX_Border, borderToBorderTag(textSpec->border),
    TAG_END);
}

static struct Gadget *buildInteger(IntegerSpec *integerSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = integerSpec->left;
  newGadget.ng_TopEdge    = integerSpec->top;
  newGadget.ng_Width      = integerSpec->width;
  newGadget.ng_Height     = integerSpec->height;
  newGadget.ng_GadgetText = integerSpec->label;
  newGadget.ng_TextAttr   = &Topaz80;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = textPlacementToFlags(integerSpec->textPlacement);
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = (void*)integerSpec->onEntry;
  return CreateGadget(INTEGER_KIND, context, &newGadget,
    GTIN_MaxChars, integerSpec->maxChars,
    GA_Disabled, stateToDisabledTag(integerSpec->state),
    TAG_END);
}

static struct Gadget *buildListView(ListViewSpec *listViewSpec, struct Gadget *context, UWORD gadgetId) {
  struct NewGadget newGadget;
  newGadget.ng_LeftEdge   = listViewSpec->left;
  newGadget.ng_TopEdge    = listViewSpec->top;
  newGadget.ng_Width      = listViewSpec->width;
  newGadget.ng_Height     = listViewSpec->height;
  newGadget.ng_GadgetText = NULL;
  newGadget.ng_TextAttr   = &Topaz80;
  newGadget.ng_GadgetID   = gadgetId;
  newGadget.ng_Flags      = 0;
  newGadget.ng_VisualInfo = getGlobalVi();
  newGadget.ng_UserData   = (void*)listViewSpec->onSelect;
  return CreateGadget(LISTVIEW_KIND, context, &newGadget,
    /* TODO: show selected */
    GTLV_Labels, listViewSpec->labels,
    TAG_END);
}

static struct Gadget *buildGadget(GadgetSpec *gadgetSpec, struct Gadget *context, UWORD gadgetId) {
  switch(gadgetSpec->kind) {
    case BUTTON_KIND:   return buildButton(gadgetSpec->specBody.buttonSpec,     context, gadgetId);
    case SCROLLER_KIND: return buildScroller(gadgetSpec->specBody.scrollerSpec, context, gadgetId);
    case STRING_KIND:   return buildString(gadgetSpec->specBody.stringSpec,     context, gadgetId);
    case TEXT_KIND:     return buildText(gadgetSpec->specBody.textSpec,         context, gadgetId);
    case INTEGER_KIND:  return buildInteger(gadgetSpec->specBody.integerSpec,   context, gadgetId);
    case LISTVIEW_KIND: return buildListView(gadgetSpec->specBody.listViewSpec, context, gadgetId);
    default:
      fprintf(stderr, "buildGadget: invalid gadget kind %lu\n", gadgetSpec->kind);
      return NULL;
  }
}

struct Gadget *buildGadgets(GadgetSpec *gadgetSpec, struct Gadget **gadgetPtr, ...) {
  va_list args;

  struct Gadget *glist = NULL;
  struct Gadget *gad = CreateContext(&glist);

  UWORD gadgetId = 0;

  va_start(args, gadgetPtr);

  do {
    gad = buildGadget(gadgetSpec, gad, gadgetId);
    *gadgetPtr = gad;

    free(gadgetSpec);

    gadgetId++;
    (gadgetSpec = va_arg(args, GadgetSpec*)) 
    && (gadgetPtr = va_arg(args, struct Gadget**));
  } while(gadgetSpec);

  va_end(args);

  if(gad) {
    return glist;
  } else {
    FreeGadgets(glist);
    return NULL;
  }
}
