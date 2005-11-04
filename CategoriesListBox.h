/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_CategoriesListBox
#define HEADER_CategoriesListBox

#include "AvailableMetersListBox.h"
#include "MetersListBox.h"
#include "DisplayOptionsListBox.h"

#include "ListBox.h"

#include "debug.h"
#include <assert.h>


typedef struct CategoriesListBox_ {
   ListBox super;

   Settings* settings;
   ScreenManager* scr;
} CategoriesListBox;




CategoriesListBox* CategoriesListBox_new(Settings* settings, ScreenManager* scr);

void CategoriesListBox_delete(Object* object);

HandlerResult CategoriesListBox_eventHandler(ListBox* super, int ch);

void CategoriesListBox_makeMetersPage(CategoriesListBox* this);

void CategoriesListBox_makeDisplayOptionsPage(CategoriesListBox* this);

void CategoriesListBox_makeColorsPage(CategoriesListBox* this);

#endif