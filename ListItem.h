/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_ListItem
#define HEADER_ListItem
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"
#include "CRT.h"

#include "debug.h"


typedef struct ListItem_ {
   Object super;
   char* text;
} ListItem;

extern char* LISTITEM_CLASS;


ListItem* ListItem_new(char* text);

void ListItem_delete(Object* cast);

void ListItem_display(Object* cast, RichString* out);

#endif
