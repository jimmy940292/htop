/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_Object
#define HEADER_Object
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "RichString.h"
#include "CRT.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "debug.h"

typedef struct Object_ Object;

typedef void(*Object_Display)(Object*, RichString*);
typedef int(*Object_Compare)(const Object*, const Object*);
typedef void(*Object_Delete)(Object*);

struct Object_ {
   char* class;
   Object_Display display;
   Object_Compare compare;
   Object_Delete delete;
};


void Object_new();

bool Object_instanceOf(Object* this, char* class);

void Object_delete(Object* this);

void Object_display(Object* this, RichString* out);

int Object_compare(const Object* this, const Object* o);

#endif
