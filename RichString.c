
#include "RichString.h"

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <assert.h>
#include <sys/param.h>

#include "debug.h"

#define RICHSTRING_MAXLEN 512

/*{

typedef struct RichString_ {
   int len;
   chtype chstr[RICHSTRING_MAXLEN+1];
} RichString;

}*/

/* private property */
WINDOW* workArea = NULL;

RichString RichString_new() {
   RichString this;
   this.len = 0;
   return this;
}

void RichString_delete(RichString this) {
}

void RichString_prune(RichString* this) {
   this->len = 0;
}

void RichString_attrOn(int attrs) {
   wattron(workArea, attrs);
}

void RichString_attrOff(int attrs) {
   wattroff(workArea, attrs);
}

void RichString_write(RichString* this, int attrs, char* data) {
   this->len = 0;
   RichString_append(this, attrs, data);
}

void RichString_append(RichString* this, int attrs, char* data) {
   if (!workArea) {
      workArea = newpad(1, RICHSTRING_MAXLEN);
   }
   assert(workArea);
   wattron(workArea, attrs);
   int len = strlen(data);
   int maxToWrite = (RICHSTRING_MAXLEN - 1) - this->len;
   int wrote = MIN(maxToWrite, len);
   mvwaddnstr(workArea, 0, 0, data, maxToWrite);
   int oldstrlen = this->len;
   this->len += wrote;
   mvwinchnstr(workArea, 0, 0, this->chstr + oldstrlen, wrote);
   wattroff(workArea, attrs);
}

void RichString_setAttr(RichString *this, int attrs) {
   for (int i = 0; i < this->len; i++) {
      char c = this->chstr[i];
      this->chstr[i] = c | attrs;
   }
}

void RichString_applyAttr(RichString *this, int attrs) {
   for (int i = 0; i < this->len; i++) {
      this->chstr[i] |= attrs;
   }
}
