/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "FunctionBar.h"
#include "CRT.h"

#include "debug.h"
#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>

/*{

typedef struct FunctionBar_ {
   int size;
   char** functions;
   char** keys;
   int* events;
} FunctionBar;

}*/

/* private property */
char* FunctionBar_FKeys[10] = {" 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10"};

/* private property */
int FunctionBar_FEvents[10] = {KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4), KEY_F(5), KEY_F(6), KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10)};

FunctionBar* FunctionBar_new(int size, char** functions, char** keys, int* events) {
   FunctionBar* this = malloc(sizeof(FunctionBar));
   this->functions = functions;
   this->size = size;
   if (keys && events) {
      this->keys = keys;
      this->events = events;
   } else {
      this->keys = FunctionBar_FKeys;
      this->events = FunctionBar_FEvents;
      assert(this->size == 10);
   }
   return this;
}

void FunctionBar_delete(FunctionBar* this) {
   // These are always static data, RIGHT? >;)
   // free(this->functions);
   // free(this->keys);
   free(this);
}

void FunctionBar_draw(FunctionBar* this, char* buffer) {
   FunctionBar_drawAttr(this, buffer, CRT_colors[FUNCTION_BAR]);
}

void FunctionBar_drawAttr(FunctionBar* this, char* buffer, int attr) {
   attrset(CRT_colors[FUNCTION_BAR]);
   mvhline(LINES-1, 0, ' ', COLS);
   int x = 0;
   for (int i = 0; i < this->size; i++) {
      attrset(CRT_colors[FUNCTION_KEY]);
      mvaddstr(LINES-1, x, this->keys[i]);
      x += strlen(this->keys[i]);
      attrset(CRT_colors[FUNCTION_BAR]);
      mvaddstr(LINES-1, x, this->functions[i]);
      x += strlen(this->functions[i]);
   }
   if (buffer != NULL) {
      attrset(attr);
      mvaddstr(LINES-1, x, buffer);
   }
   attrset(CRT_colors[RESET_COLOR]);
}

int FunctionBar_synthesizeEvent(FunctionBar* this, int pos) {
   int x = 0;
   for (int i = 0; i < this->size; i++) {
      x += strlen(this->keys[i]);
      x += strlen(this->functions[i]);
      if (pos < x) {
         return this->events[i];
      }
   }
   return ERR;
}
