#ifndef FRAMEWORK_GADGETS_H
#define FRAMEWORK_GADGETS_H

#include <libraries/gadtools.h>

#include "window.h"

typedef void (*OnClick)(FrameworkWindow*);
typedef void (*OnEntry)(FrameworkWindow*);

typedef enum Border_tag {
  NO_BORDER,
  BORDERED
} Border;

typedef enum Orientation_tag {
  HORIZONTAL,
  VERTICAL
} Orientation;

typedef enum State_tag {
  DISABLED,
  ENABLED
} State;

typedef enum TextPlacement_tag {
  TEXT_ON_LEFT,
  TEXT_ON_RIGHT,
  TEXT_ABOVE,
  TEXT_BELOW,
  TEXT_INSIDE
} TextPlacement;

typedef struct ButtonSpec_tag {
  WORD left, top;
  WORD width, height;
  char *label;
  TextPlacement textPlacement;
  State state;

  OnClick onClick;
} ButtonSpec;

typedef struct ScrollSpec_tag {
  WORD left, top;
  WORD width, height;
  State state;
  Orientation orientation;
} ScrollerSpec;

typedef struct StringSpec_tag {
  WORD left, top;
  WORD width, height;
  char *label;
  TextPlacement textPlacement;
  State state;
  OnEntry onEntry;
} StringSpec;

typedef struct TextSpec_tag {
  WORD left, top;
  WORD width, height;
  char *label;
  TextPlacement textPlacement;
  char *text;
  Border border;
} TextSpec;

typedef struct ListViewSpec_tag {
  WORD left, top;
  WORD width, height;
  struct List *labels;
/* TODO: show selected */
} ListViewSpec;

typedef struct GadgetSpec_tag GadgetSpec;

GadgetSpec *makeButtonGadget(ButtonSpec*);
GadgetSpec *makeScrollerGadget(ScrollerSpec*);
GadgetSpec *makeStringGadget(StringSpec*);
GadgetSpec *makeTextGadget(TextSpec*);
GadgetSpec *makeListViewGadget(ListViewSpec*);

/* pass in pairs of GadgetSpecs and where to build the gadgets */
/* returns a pointer to the first gadget */
struct Gadget *buildGadgets(GadgetSpec*, struct Gadget**, ...);

#endif
