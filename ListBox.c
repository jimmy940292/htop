/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"
#include "ListBox.h"
#include "TypedVector.h"
#include "CRT.h"
#include "RichString.h"

#include "debug.h"

#include <assert.h>
#include <math.h>
#include <sys/param.h>
#include <stdbool.h>

#include <curses.h>
//#link curses

/*{
typedef struct ListBox_ {
   int x, y, w, h;
   WINDOW* window;
   TypedVector* items;
   int selected;
   int scrollV, scrollH;
   int oldSelected;
   bool needsRedraw;
   RichString header;
} ListBox;
}*/

ListBox* ListBox_new(int x, int y, int w, int h, char* type) {
   ListBox* this;
   this = malloc(sizeof(ListBox));
   this->x = x;
   this->y = y;
   this->w = w;
   this->h = h;
   this->items = TypedVector_new(type, false);
   this->scrollV = 0;
   this->scrollH = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->needsRedraw = true;
   this->header.len = 0;
   return this;
}

void ListBox_setHeader(ListBox* this, RichString header) {
   assert (this != NULL);

   if (this->header.len == 0) {
      this->y++;
      this->h--;
   } else {
      RichString_delete(this->header);
   }
   this->header = header;
   this->needsRedraw = true;
}

void ListBox_delete(ListBox* this) {
   assert (this != NULL);

   RichString_delete(this->header);
   TypedVector_delete(this->items);
   free(this);
}

void ListBox_move(ListBox* this, int x, int y) {
   assert (this != NULL);

   if (this->header.len > 0)
      y++;
   this->x = x;
   this->y = y;
   this->needsRedraw = true;
}

void ListBox_resize(ListBox* this, int w, int h) {
   assert (this != NULL);

   if (this->header.len > 0)
      h--;
   this->w = w;
   this->h = h;
   this->needsRedraw = true;
}

void ListBox_prune(ListBox* this) {
   assert (this != NULL);

   TypedVector_prune(this->items);
   this->scrollV = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->needsRedraw = true;
}

void ListBox_add(ListBox* this, Object* o) {
   assert (this != NULL);

   TypedVector_add(this->items, o);
}

void ListBox_set(ListBox* this, int i, Object* o) {
   assert (this != NULL);

   TypedVector_set(this->items, i, o);
}

Object* ListBox_get(ListBox* this, int i) {
   assert (this != NULL);

   return TypedVector_get(this->items, i);
}

Object* ListBox_getSelected(ListBox* this) {
   assert (this != NULL);

   return TypedVector_get(this->items, this->selected);
}

int ListBox_getSelectedIndex(ListBox* this) {
   assert (this != NULL);

   return this->selected;
}

int ListBox_getSize(ListBox* this) {
   assert (this != NULL);

   return TypedVector_size(this->items);
}

void ListBox_setSelected(ListBox* this, int selected) {
   assert (this != NULL);

   selected = MIN(TypedVector_size(this->items), selected);
   this->selected = selected;
}

void ListBox_draw(ListBox* this) {
   assert (this != NULL);

   int first, last;
   int itemCount = TypedVector_size(this->items);
   int scrollH = this->scrollH;
   int y = this->y; int x = this->x;
   first = this->scrollV;

   if (this->h > itemCount) {
      last = this->scrollV + itemCount;
      move(y + last, x + 0);
   } else {
      last = MIN(itemCount, this->scrollV + this->h);
   }
   if (this->selected < first) {
      first = this->selected;
      this->scrollV = first;
      this->needsRedraw = true;
   }
   if (this->selected >= last) {
      last = MIN(itemCount, this->selected + 1);
      first = MAX(0, last - this->h);
      this->scrollV = first;
      this->needsRedraw = true;
   }
   assert(first >= 0);
   assert(last <= itemCount);
   if (this->needsRedraw) {

      if (this->header.len > 0) {
         attron(COLOR_PAIR(HEADER_PAIR));
         RichString_applyAttr(&this->header, COLOR_PAIR(HEADER_PAIR));
         move(y - 1, x);
         hline(' ', this->w);
         if (scrollH < this->header.len) {
            mvaddchstr(y - 1, x, this->header.chstr + scrollH);
         }
         attroff(COLOR_PAIR(HEADER_PAIR));
      }

      for(int i = first, j = 0; j < this->h && i < last; i++, j++) {
         Object* itemObj = TypedVector_get(this->items, i);
         RichString itemRef = RichString_new();
         itemObj->display(itemObj, &itemRef);
         if (i == this->selected) {
            attron(COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
            RichString_setAttr(&itemRef, COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
            mvhline(y + j, x+0, ' ', this->w);
            if (scrollH < itemRef.len)
               mvaddchstr(y+j, x+0, itemRef.chstr + scrollH);
            attroff(COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
         } else {
            mvhline(y+j, x+0, ' ', this->w);
            if (scrollH < itemRef.len)
               mvaddchstr(y+j, x+0, itemRef.chstr + scrollH);
         }
      }
      for (int i = y + (last - first); i < y + this->h; i++)
         mvhline(i, x+0, ' ', this->w);
      this->needsRedraw = false;

   } else {
      Object* oldObj = TypedVector_get(this->items, this->oldSelected);
      RichString oldRef = RichString_new();
      oldObj->display(oldObj, &oldRef);
      Object* newObj = TypedVector_get(this->items, this->selected);
      RichString newRef = RichString_new();
      newObj->display(newObj, &newRef);
      mvhline(y+ this->oldSelected - this->scrollV, x+0, ' ', this->w);
      if (scrollH < oldRef.len)
         mvaddchstr(y+ this->oldSelected - this->scrollV, x+0, oldRef.chstr + this->scrollH);
      attron(COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
      mvhline(y+this->selected - this->scrollV, x+0, ' ', this->w);
      RichString_setAttr(&newRef, COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
      if (scrollH < newRef.len)
         mvaddchstr(y+this->selected - this->scrollV, x+0, newRef.chstr + this->scrollH);
      attroff(COLOR_PAIR(INVERSE_PAIR) | A_REVERSE);
   }
   this->oldSelected = this->selected;
   move(0, 0);
}

void ListBox_onKey(ListBox* this, int key) {
   assert (this != NULL);
   switch (key) {
   case KEY_DOWN:
      if (this->selected + 1 < TypedVector_size(this->items))
         this->selected++;
      break;
   case KEY_UP:
      if (this->selected > 0)
         this->selected--;
      break;
   case KEY_LEFT:
      if (this->scrollH > 0) {
         this->scrollH -= 5;
         this->needsRedraw = true;
      }
      break;
   case KEY_RIGHT:
      this->scrollH += 5;
      this->needsRedraw = true;
      break;
   case KEY_PPAGE:
      this->selected -= this->h;
      if (this->selected < 0)
         this->selected = 0;
      break;
   case KEY_NPAGE:
      this->selected += this->h;
      int size = TypedVector_size(this->items);
      if (this->selected >= size)
         this->selected = size - 1;
      break;
   case KEY_HOME:
      this->selected = 0;
      break;
   case KEY_END:
      this->selected = TypedVector_size(this->items) - 1;
      break;
   }
}
