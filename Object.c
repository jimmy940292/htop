/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"
#include "RichString.h"
#include "CRT.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "debug.h"

/*{
typedef struct Object_ Object;

typedef void(*Method_Object_display)(Object*, RichString*);
typedef bool(*Method_Object_equals)(const Object*, const Object*);
typedef void(*Method_Object_delete)(Object*);

struct Object_ {
   char* class;
   Method_Object_display display;
   Method_Object_equals equals;
   Method_Object_delete delete;
};
}*/

/* private property */
char* OBJECT_CLASS = "Object";

void Object_new() {
   Object* this;
   this = malloc(sizeof(Object));
   this->class = OBJECT_CLASS;
   this->display = Object_display;
   this->equals = Object_equals;
   this->delete = Object_delete;
}

bool Object_instanceOf(Object* this, char* class) {
   return this->class == class;
}

void Object_delete(Object* this) {
   free(this);
}

void Object_display(Object* this, RichString* out) {
   char objAddress[50];
   sprintf(objAddress, "%s @ %p", this->class, (void*) this);
   RichString_write(out, CRT_colors[DEFAULT_COLOR], objAddress);
}

bool Object_equals(const Object* this, const Object* o) {
   return (this == o);
}
