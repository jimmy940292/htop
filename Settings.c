/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Settings.h"
#include "String.h"
#include "ProcessFilter.h"

/*{

typedef struct Settings_ {
   ProcessFilter* filter;
   char* userSettings;
} Settings;

}*/

Settings* Settings_new(ProcessFilter* pf) {
   Settings* this = malloc(sizeof(Settings));
   this->filter = pf;
   // TODO: how to get SYSCONFDIR correctly through Autoconf?
   // char* systemSettings = String_cat(SYSCONFDIR, "/htoprc");
   // Settings_read(this, systemSettings);
   char* home = getenv("HOME");
   this->userSettings = String_cat(home, "/.htoprc");
   Settings_read(this, this->userSettings);
   // free(systemSettings);
   return this;
}

void Settings_delete(Settings* this) {
   free(this->userSettings);
   free(this);
}

bool Settings_read(Settings* this, char* fileName) {
   // TODO: implement File object and make
   // file I/O object-oriented.
   FILE* fd;
   fd = fopen(fileName, "r");
   if (fd == NULL) {
      return false;
   }
   const int maxLine = 512;
   char buffer[maxLine];

   while (!feof(fd)) {
      fgets(buffer, maxLine, fd);
      char** option = String_split(buffer, '=');
      // fields
      if (String_eq(option[0], "fields")) {
         char* trim = String_trim(option[1]);
         char** ids = String_split(trim, ' ');
         free(trim);
         int i;
         for (i = 0; ids[i] != NULL; i++) {
            this->filter->fields[i] = atoi(ids[i]);
         }
         this->filter->fields[i-1] = LAST;
         String_freeArray(ids);
      // sort_key
      } else if (String_eq(option[0], "sort_key")) {
         this->filter->sortKey = atoi(option[1]);
      // sort_direction
      } else if (String_eq(option[0], "sort_direction")) {
         this->filter->direction = atoi(option[1]);
      }
      String_freeArray(option);
   }
   fclose(fd);
   return true;
}

bool Settings_write(Settings* this) {
   // TODO: implement File object and make
   // file I/O object-oriented.
   FILE* fd;
   fd = fopen(this->userSettings, "w");
   if (fd == NULL) {
      return false;
   }
   // fields
   fprintf(fd, "fields=");
   for (int i = 0; this->filter->fields[i] != LAST; i++) {
      fprintf(fd, "%d ", (int) this->filter->fields[i]);
   }
   fprintf(fd, "\n");
   // sort_key
   fprintf(fd, "sort_key=%d\n", (int) this->filter->sortKey);
   // sort_direction
   fprintf(fd, "sort_direction=%d\n", (int) this->filter->direction);
   
   fclose(fd);
   return true;
}
