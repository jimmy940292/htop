/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_MetersListBox
#define HEADER_MetersListBox


#include "ListBox.h"
#include "Settings.h"
#include "ScreenManager.h"

#include "debug.h"
#include <assert.h>


typedef struct MetersListBox_ {
   ListBox super;

   Settings* settings;
   TypedVector* meters;
   ScreenManager* scr;
} MetersListBox;


MetersListBox* MetersListBox_new(Settings* settings, char* header, TypedVector* meters, ScreenManager* scr);

void MetersListBox_delete(Object* object);

HandlerResult MetersListBox_eventHandler(ListBox* super, int ch);

#endif
