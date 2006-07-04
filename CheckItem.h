/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_CheckItem
#define HEADER_CheckItem
/*
htop
(C) 2004-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"
#include "CRT.h"

#include "debug.h"


typedef struct CheckItem_ {
   Object super;
   char* text;
   bool* value;
} CheckItem;

extern char* CHECKITEM_CLASS;


CheckItem* CheckItem_new(char* text, bool* value);

void CheckItem_delete(Object* cast);

void CheckItem_display(Object* cast, RichString* out);

#endif
