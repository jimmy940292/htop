/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ListItem.h"
#include "Object.h"
#include "CRT.h"

#include "debug.h"

/*{

typedef struct ListItem_ {
   Object super;
   char* text;
} ListItem;

extern char* LISTITEM_CLASS;
}*/

/* private property */
char* LISTITEM_CLASS = "ListItem";

ListItem* ListItem_new(char* text) {
   ListItem* this = malloc(sizeof(ListItem));
   ((Object*)this)->class = LISTITEM_CLASS;
   ((Object*)this)->display = ListItem_display;
   ((Object*)this)->delete = ListItem_delete;
   this->text = text;
   return this;
}

void ListItem_delete(Object* cast) {
   ListItem* this = (ListItem*)cast;
   assert (this != NULL);

   free(this->text);
   free(this);
}

void ListItem_display(Object* cast, RichString* out) {
   ListItem* this = (ListItem*)cast;
   assert (this != NULL);
   RichString_write(out, CRT_colors[DEFAULT_COLOR], this->text);
}
