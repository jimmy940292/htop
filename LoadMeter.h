/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_LoadMeter
#define HEADER_LoadMeter
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Meter.h"

#include "ProcessList.h"

#include "debug.h"


typedef struct LoadMeter_ LoadMeter;

struct LoadMeter_ {
   Meter super;
   ProcessList* pl;
};


LoadMeter* LoadMeter_new();


void LoadMeter_setValues(Meter* cast);

void LoadMeter_display(Object* cast, RichString* out);

#endif
