/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Meter.h"
#include "Object.h"
#include "CRT.h"
#include "ListItem.h"
#include "String.h"

#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

#include "debug.h"
#include <assert.h>

#define METER_BARBUFFER_LEN 128
#define METER_GRAPHBUFFER_LEN 128

/*{

typedef struct Meter_ Meter;

typedef void(*Method_Meter_setValues)(Meter*);
typedef void(*Method_Meter_draw)(Meter*, int, int, int);

typedef enum MeterMode_ {
   UNSET,
   BAR,
   TEXT,
   GRAPH,
   LED,
   LAST_METERMODE
} MeterMode;

struct Meter_ {
   Object super;
   
   int h;
   int w;
   Method_Meter_draw draw;
   Method_Meter_setValues setValues;
   int items;
   int* attributes;
   double* values;
   double total;
   char* caption;
   char* name;
   union {
      RichString* rs;
      char* c;
      double* graph;
   } displayBuffer;
   MeterMode mode;
};

extern char* METER_CLASS;

}*/

/* private property */
char* METER_CLASS = "Meter";

/* private */
char* Meter_ledDigits[3][10] = {
   { " __ ","    "," __ "," __ ","    "," __ "," __ "," __ "," __ "," __ "},
   { "|  |","   |"," __|"," __|","|__|","|__ ","|__ ","   |","|__|","|__|"},
   { "|__|","   |","|__ "," __|","   |"," __|","|__|","   |","|__|"," __|"},
};

/* private property */
char Meter_barCharacters[] = "|#*@$%&";

Meter* Meter_new(char* name, char* caption, int items) {
   Meter* this = malloc(sizeof(Meter));
   Meter_init(this, name, caption, items);
   return this;
}

void Meter_init(Meter* this, char* name, char* caption, int items) {
   ((Object*)this)->delete = Meter_delete;
   ((Object*)this)->class = METER_CLASS;
   this->items = items;
   this->name = name;
   this->caption = caption;
   this->attributes = malloc(sizeof(int) * items);
   this->values = malloc(sizeof(double) * items);
   this->displayBuffer.c = NULL;
   this->mode = UNSET;
   this->h = 0;
}

void Meter_delete(Object* cast) {
   Meter* this = (Meter*) cast;
   assert (this != NULL);
   Meter_done(this);
   free(this);
}

/* private */
void Meter_freeBuffer(Meter* this) {
   switch (this->mode) {
   case BAR: {
      free(this->displayBuffer.c);
      break;
   }
   case LED:
   case TEXT: {
      free(this->displayBuffer.rs);
      break;
   }
   case GRAPH: {
      free(this->displayBuffer.graph);
      break;
   }
   default: {
   }
   }
   this->h = 0;
}

void Meter_done(Meter* this) {
   free(this->caption);
   free(this->attributes);
   free(this->values);
   free(this->name);
   Meter_freeBuffer(this);
}

/* private */
void Meter_drawBar(Meter* this, int x, int y, int w) {
   
   w -= 2;
   attrset(CRT_colors[METER_TEXT]);
   mvaddstr(y, x, this->caption);
   int captionLen = strlen(this->caption);
   x += captionLen;
   w -= captionLen;
   attrset(CRT_colors[BAR_BORDER]);
   mvaddch(y, x, '[');
   mvaddch(y, x + w, ']');
   
   w--;
   x++;
   char bar[w];
   
   this->setValues(this);
   
   int blockSizes[10];
   for (int i = 0; i < w; i++)
      bar[i] = ' ';

   sprintf(bar + (w-strlen(this->displayBuffer.c)), "%s", this->displayBuffer.c);

   // First draw in the bar[] buffer...
   double total = 0.0;
   int offset = 0;
   for (int i = 0; i < this->items; i++) {
      this->values[i] = MAX(this->values[i], 0);
      this->values[i] = MIN(this->values[i], this->total);
      double value = this->values[i];
      if (value > 0) {
         blockSizes[i] = ceil((value/this->total) * w);
      } else {
         blockSizes[i] = 0;
      }
      int nextOffset = offset + blockSizes[i];
      // (Control against invalid values)
      nextOffset = MAX(nextOffset, 0);
      nextOffset = MIN(nextOffset, w);
      for (int j = offset; j < nextOffset; j++)
         if (bar[j] == ' ') {
            if (CRT_hasColors) {
               bar[j] = '|';
            } else {
               bar[j] = Meter_barCharacters[i];
            }
         }
      offset = nextOffset;
      total += this->values[i];
   }

   // ...then print the buffer.
   offset = 0;
   for (int i = 0; i < this->items; i++) {
      attrset(this->attributes[i]);
      mvaddnstr(y, x + offset, bar + offset, blockSizes[i]);
      offset += blockSizes[i];
      offset = MAX(offset, 0);
      offset = MIN(offset, w);
   }
   if (offset < w) {
      attrset(CRT_colors[BAR_SHADOW]);
      mvaddnstr(y, x + offset, bar + offset, w - offset);
   }

   move(y, x + w + 1);
   attrset(CRT_colors[RESET_COLOR]);
}

/* private */
void Meter_drawText(Meter* this, int x, int y, int w) {
   this->setValues(this);
   this->w = w;
   attrset(CRT_colors[METER_TEXT]);
   mvaddstr(y, x, this->caption);
   int captionLen = strlen(this->caption);
   w -= captionLen;
   x += captionLen;
   ((Object*)this)->display((Object*)this, this->displayBuffer.rs);
   mvhline(y, x, ' ', CRT_colors[DEFAULT_COLOR]);
   attrset(CRT_colors[RESET_COLOR]);
   mvaddchstr(y, x, this->displayBuffer.rs->chstr);
}

/* private */
void Meter_drawDigit(int x, int y, int n) {
   for (int i = 0; i < 3; i++) {
      mvaddstr(y+i, x, Meter_ledDigits[i][n]);
   }
}

/* private */
void Meter_drawLed(Meter* this, int x, int y, int w) {
   this->setValues(this);
   ((Object*)this)->display((Object*)this, this->displayBuffer.rs);
   attrset(CRT_colors[LED_COLOR]);
   mvaddstr(y+2, x, this->caption);
   int xx = x + strlen(this->caption);
   for (int i = 0; i < this->displayBuffer.rs->len; i++) {
      char c = this->displayBuffer.rs->chstr[i];
      if (c >= '0' && c <= '9') {
         Meter_drawDigit(xx, y, c-48);
	 xx += 4;
      } else {
         mvaddch(y+2, xx, c);
         xx += 1;
      }
   }
   attrset(CRT_colors[RESET_COLOR]);
}

#define DrawDot(a,y,c) do { \
   attrset(a); \
   mvaddstr(y, x+k, c); \
} while(0)

/* private */
void Meter_drawGraph(Meter* this, int x, int y, int w) {

   for (int i = 0; i < METER_GRAPHBUFFER_LEN - 1; i++) {
      this->displayBuffer.graph[i] = this->displayBuffer.graph[i+1];
   }

   this->setValues(this);

   double value = 0.0;
   for (int i = 0; i < this->items; i++)
      value += this->values[i] / this->total;
   this->displayBuffer.graph[METER_GRAPHBUFFER_LEN - 1] = value;

   for (int i = METER_GRAPHBUFFER_LEN - w, k = 0; i < METER_GRAPHBUFFER_LEN; i++, k++) {
      double value = this->displayBuffer.graph[i];
      DrawDot( CRT_colors[DEFAULT_COLOR], y, " " );
      DrawDot( CRT_colors[DEFAULT_COLOR], y+1, " " );
      DrawDot( CRT_colors[DEFAULT_COLOR], y+2, " " );
      if (value >= 1.00)      DrawDot( CRT_colors[GRAPH_1], y, "^" );
      else if (value >= 0.95) DrawDot( CRT_colors[GRAPH_1], y, "`" );
      else if (value >= 0.90) DrawDot( CRT_colors[GRAPH_1], y, "'" );
      else if (value >= 0.85) DrawDot( CRT_colors[GRAPH_2], y, "-" );
      else if (value >= 0.80) DrawDot( CRT_colors[GRAPH_2], y, "." );
      else if (value >= 0.75) DrawDot( CRT_colors[GRAPH_2], y, "," );
      else if (value >= 0.70) DrawDot( CRT_colors[GRAPH_3], y, "_" );
      else if (value >= 0.65) DrawDot( CRT_colors[GRAPH_3], y+1, "~" );
      else if (value >= 0.60) DrawDot( CRT_colors[GRAPH_3], y+1, "`" );
      else if (value >= 0.55) DrawDot( CRT_colors[GRAPH_4], y+1, "'" );
      else if (value >= 0.50) DrawDot( CRT_colors[GRAPH_4], y+1, "-" );
      else if (value >= 0.45) DrawDot( CRT_colors[GRAPH_4], y+1, "." );
      else if (value >= 0.40) DrawDot( CRT_colors[GRAPH_5], y+1, "," );
      else if (value >= 0.35) DrawDot( CRT_colors[GRAPH_5], y+1, "_" );
      else if (value >= 0.30) DrawDot( CRT_colors[GRAPH_6], y+2, "~" );
      else if (value >= 0.25) DrawDot( CRT_colors[GRAPH_7], y+2, "`" );
      else if (value >= 0.20) DrawDot( CRT_colors[GRAPH_7], y+2, "'" );
      else if (value >= 0.15) DrawDot( CRT_colors[GRAPH_7], y+2, "-" );
      else if (value >= 0.10) DrawDot( CRT_colors[GRAPH_8], y+2, "." );
      else if (value >= 0.05) DrawDot( CRT_colors[GRAPH_8], y+2, "," );
      else                    DrawDot( CRT_colors[GRAPH_9], y+2, "_" );
   }
   attrset(CRT_colors[RESET_COLOR]);
}

void Meter_setMode(Meter* this, MeterMode mode) {
   Meter_freeBuffer(this);
   switch (mode) {
   case UNSET: {
      // fallthrough to a sane default.
      mode = TEXT;
   }
   case TEXT: {
      this->draw = Meter_drawText;
      this->displayBuffer.rs = malloc(sizeof(RichString));
      this->h = 1;
      break;
   }
   case LED: {
      this->draw = Meter_drawLed;
      this->displayBuffer.rs = malloc(sizeof(RichString));
      this->h = 3;
      break;
   }
   case BAR: {
      this->draw = Meter_drawBar;
      this->displayBuffer.c = malloc(METER_BARBUFFER_LEN);
      this->h = 1;
      break;
   }
   case GRAPH: {
      this->draw = Meter_drawGraph;
      this->displayBuffer.c = calloc(METER_GRAPHBUFFER_LEN, sizeof(double));
      this->h = 3;
      break;
   }
   default: {
      assert(false);
   }
   }
   this->mode = mode;
}

ListItem* Meter_toListItem(Meter* this) {
   char buffer[50]; char* mode = NULL;
   switch (this->mode) {
   case BAR: mode = "Bar"; break;
   case LED: mode = "LED"; break;
   case TEXT: mode = "Text"; break;
   case GRAPH: mode = "Graph"; break;
   default: {
      assert(false);
   }
   }
   sprintf(buffer, "%s [%s]", this->name, mode);
   return ListItem_new(String_copy(buffer)); 
}
