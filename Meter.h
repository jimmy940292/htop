/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_Meter
#define HEADER_Meter
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

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
   int** attributes;
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





Meter* Meter_new(char* name, char* caption, int items);

void Meter_init(Meter* this, char* name, char* caption, int items);

void Meter_delete(Object* cast);


void Meter_done(Meter* this);





#define DrawDot(a,y,c) do { \
   attrset(a); \
   mvaddstr(y, x+k, c); \
} while(0)


void Meter_setMode(Meter* this, MeterMode mode);

ListItem* Meter_toListItem(Meter* this);

#endif
