
#include "CategoriesListBox.h"
#include "AvailableMetersListBox.h"
#include "MetersListBox.h"
#include "DisplayOptionsListBox.h"
#include "ColorsListBox.h"

#include "ListBox.h"

#include "debug.h"
#include <assert.h>

/*{

typedef struct CategoriesListBox_ {
   ListBox super;

   Settings* settings;
   ScreenManager* scr;
} CategoriesListBox;

}*/

/* private property */
char* MetersFunctions[10] = {"      ", "      ", "      ", "Type  ", "Add L ", "Add R ", "MoveUp", "MoveDn", "Remove", "Done  "};

/* private property */
char* DisplayOptionsFunctions[10] = {"      ", "      ", "      ", "      ", "      ", "      ", "      ", "      ", "      ", "Done  "};

/* private property */
char* ColorsFunctions[10] = {"      ", "      ", "      ", "      ", "      ", "      ", "      ", "      ", "      ", "Done  "};

CategoriesListBox* CategoriesListBox_new(Settings* settings, ScreenManager* scr) {
   CategoriesListBox* this = (CategoriesListBox*) malloc(sizeof(CategoriesListBox));
   ListBox* super = (ListBox*) this;
   ListBox_init(super, 1, 1, 1, 1, LISTITEM_CLASS, true);
   ((Object*)this)->delete = CategoriesListBox_delete;

   this->settings = settings;
   this->scr = scr;
   super->eventHandler = CategoriesListBox_eventHandler;
   ListBox_setHeader(super, RichString_quickString(CRT_colors[PANEL_HEADER_FOCUS], "Setup"));
   ListBox_add(super, (Object*) ListItem_new(String_copy("Meters")));
   ListBox_add(super, (Object*) ListItem_new(String_copy("Display options")));
   ListBox_add(super, (Object*) ListItem_new(String_copy("Colors")));
   return this;
}

void CategoriesListBox_delete(Object* object) {
   ListBox* super = (ListBox*) object;
   CategoriesListBox* this = (CategoriesListBox*) object;
   ListBox_done(super);
   free(this);
}

HandlerResult CategoriesListBox_eventHandler(ListBox* super, int ch) {
   CategoriesListBox* this = (CategoriesListBox*) super;

   HandlerResult result = IGNORED;

   int previous = ListBox_getSelectedIndex(super);

   switch (ch) {
      case KEY_UP:
      case KEY_DOWN:
      case KEY_NPAGE:
      case KEY_PPAGE:
      case KEY_HOME:
      case KEY_END: {
         ListBox_onKey(super, ch);
         int selected = ListBox_getSelectedIndex(super);
         if (previous != selected) {
            int size = ScreenManager_size(this->scr);
            for (int i = 1; i < size; i++)
               ScreenManager_remove(this->scr, 1);
            switch (selected) {
               case 0:
                  CategoriesListBox_makeMetersPage(this);
                  break;
               case 1:
                  CategoriesListBox_makeDisplayOptionsPage(this);
                  break;
               case 2:
                  CategoriesListBox_makeColorsPage(this);
                  break;
            }
         }
         result = HANDLED;
      }
   }

   return result;
}

// TODO: factor out common code from these functions into a generic makePage

void CategoriesListBox_makeMetersPage(CategoriesListBox* this) {
   FunctionBar* fuBar = FunctionBar_new(10, MetersFunctions, NULL, NULL);
   ListBox* lbLeftMeters = (ListBox*) MetersListBox_new(this->settings, "Left column", this->settings->header->leftMeters, this->scr);
   ListBox* lbRightMeters = (ListBox*) MetersListBox_new(this->settings, "Right column", this->settings->header->rightMeters, this->scr);
   ListBox* lbAvailableMeters = (ListBox*) AvailableMetersListBox_new(this->settings, lbLeftMeters, lbRightMeters, this->scr);
   ScreenManager_add(this->scr, lbLeftMeters, 20);
   ScreenManager_add(this->scr, lbRightMeters, 20);
   ScreenManager_add(this->scr, lbAvailableMeters, -1);
   ScreenManager_setFunctionBar(this->scr, fuBar);
}

void CategoriesListBox_makeDisplayOptionsPage(CategoriesListBox* this) {
   FunctionBar* fuBar = FunctionBar_new(10, DisplayOptionsFunctions, NULL, NULL);
   ListBox* lbDisplayOptions = (ListBox*) DisplayOptionsListBox_new(this->settings, this->scr);
   ScreenManager_add(this->scr, lbDisplayOptions, -1);
   ScreenManager_setFunctionBar(this->scr, fuBar);
}

void CategoriesListBox_makeColorsPage(CategoriesListBox* this) {
   FunctionBar* fuBar = FunctionBar_new(10, ColorsFunctions, NULL, NULL);
   ListBox* lbColors = (ListBox*) ColorsListBox_new(this->settings, this->scr);
   ScreenManager_add(this->scr, lbColors, -1);
   ScreenManager_setFunctionBar(this->scr, fuBar);
}
