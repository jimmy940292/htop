/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_ScreenManager
#define HEADER_ScreenManager
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ListBox.h"
#include "Object.h"
#include "TypedVector.h"
#include "FunctionBar.h"

#include "debug.h"
#include <assert.h>

#include <stdbool.h>


typedef enum Orientation_ {
   VERTICAL,
   HORIZONTAL
} Orientation;

typedef struct ScreenManager_ {
   int x1;
   int y1;
   int x2;
   int y2;
   Orientation orientation;
   TypedVector* items;
   int itemCount;
   FunctionBar* fuBar;
   bool owner;
} ScreenManager;


ScreenManager* ScreenManager_new(int x1, int y1, int x2, int y2, Orientation orientation, bool owner);

void ScreenManager_delete(ScreenManager* this);

inline int ScreenManager_size(ScreenManager* this);

void ScreenManager_add(ScreenManager* this, ListBox* item, int size);

ListBox* ScreenManager_remove(ScreenManager* this, int index);

void ScreenManager_setFunctionBar(ScreenManager* this, FunctionBar* fuBar);

void ScreenManager_resize(ScreenManager* this, int x1, int y1, int x2, int y2);

void ScreenManager_run(ScreenManager* this, ListBox** lastFocus, int* lastKey);

#endif
