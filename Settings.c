/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Settings.h"
#include "String.h"
#include "ProcessList.h"
#include "Header.h"

#include "debug.h"

/*{

typedef struct Settings_ {
   char* userSettings;
   ProcessList* pl;
   Header* header;
} Settings;

}*/

Settings* Settings_new(ProcessList* pl, Header* header) {
   Settings* this = malloc(sizeof(Settings));
   this->pl = pl;
   this->header = header;
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

/* private */
void Settings_readMeters(Settings* this, char* line, HeaderSide side) {
   char* trim = String_trim(line);
   char** ids = String_split(trim, ' ');
   free(trim);
   int i;
   for (i = 0; ids[i] != NULL; i++) {
      Header_createMeter(this->header, ids[i], side);
   }
   String_freeArray(ids);
}

/* private */
void Settings_readMeterModes(Settings* this, char* line, HeaderSide side) {
   char* trim = String_trim(line);
   char** ids = String_split(trim, ' ');
   free(trim);
   int i;
   for (i = 0; ids[i] != NULL; i++) {
      int mode = atoi(ids[i]);
      Header_setMode(this->header, i, mode, side);
   }
   String_freeArray(ids);
}

bool Settings_read(Settings* this, char* fileName) {
   // TODO: implement File object and make
   // file I/O object-oriented.
   FILE* fd;
   fd = fopen(fileName, "r");
   if (fd == NULL) {
      Header_defaultMeters(this->header);
      return false;
   }
   const int maxLine = 512;
   char buffer[maxLine];
   bool readMeters = false;
   while (!feof(fd)) {
      buffer[0] = '\0';
      fgets(buffer, maxLine, fd);
      char** option = String_split(buffer, '=');
      // fields
      if (String_eq(option[0], "fields")) {
         char* trim = String_trim(option[1]);
         char** ids = String_split(trim, ' ');
         free(trim);
         int i;
         for (i = 0; ids[i] != NULL; i++) {
            this->pl->fields[i] = atoi(ids[i]);
         }
         this->pl->fields[i] = LAST_PROCESSFIELD;
         String_freeArray(ids);
      // sort_key
      } else if (String_eq(option[0], "sort_key")) {
         this->pl->sortKey = atoi(option[1]);
      // sort_direction
      } else if (String_eq(option[0], "sort_direction")) {
         this->pl->direction = atoi(option[1]);
      // tree_view
      } else if (String_eq(option[0], "tree_view")) {
         this->pl->treeView = atoi(option[1]);
      // hide_threads
      } else if (String_eq(option[0], "hide_threads")) {
         this->pl->hideThreads = atoi(option[1]);
      // hide_kernel_threads
      } else if (String_eq(option[0], "hide_kernel_threads")) {
         this->pl->hideKernelThreads = atoi(option[1]);
      // shadow_other_users
      } else if (String_eq(option[0], "shadow_other_users")) {
         this->pl->shadowOtherUsers = atoi(option[1]);
      // highlight_base_name
      } else if (String_eq(option[0], "highlight_base_name")) {
         this->pl->highlightBaseName = atoi(option[1]);
      // highlight_megabytes
      } else if (String_eq(option[0], "highlight_megabytes")) {
         this->pl->highlightMegabytes = atoi(option[1]);
      // header_margin
      } else if (String_eq(option[0], "header_margin")) {
         this->header->margin = atoi(option[1]);
      // left_meters
      } else if (String_eq(option[0], "left_meters")) {
	 Settings_readMeters(this, option[1], LEFT_HEADER);
	 readMeters = true;
      // right_meters
      } else if (String_eq(option[0], "right_meters")) {
	 Settings_readMeters(this, option[1], RIGHT_HEADER);
	 readMeters = true;
      // left_meter_modes
      } else if (String_eq(option[0], "left_meter_modes")) {
	 Settings_readMeterModes(this, option[1], LEFT_HEADER);
	 readMeters = true;
      // right_meter_modes
      } else if (String_eq(option[0], "right_meter_modes")) {
	 Settings_readMeterModes(this, option[1], RIGHT_HEADER);
	 readMeters = true;
      }
      String_freeArray(option);
   }
   fclose(fd);
   if (!readMeters) {
      Header_defaultMeters(this->header);
   }
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
   fprintf(fd, "# Beware! This file is rewritten every time htop exits.\n");
   fprintf(fd, "# The parser is also very primitive, and not human-friendly.\n");
   fprintf(fd, "# (I know, it's in the todo list).\n");
   fprintf(fd, "fields=");
   for (int i = 0; this->pl->fields[i] != LAST_PROCESSFIELD; i++) {
      fprintf(fd, "%d ", (int) this->pl->fields[i]);
   }
   fprintf(fd, "\n");
   fprintf(fd, "sort_key=%d\n", (int) this->pl->sortKey);
   fprintf(fd, "sort_direction=%d\n", (int) this->pl->direction);
   fprintf(fd, "hide_threads=%d\n", (int) this->pl->hideThreads);
   fprintf(fd, "hide_kernel_threads=%d\n", (int) this->pl->hideKernelThreads);
   fprintf(fd, "shadow_other_users=%d\n", (int) this->pl->shadowOtherUsers);
   fprintf(fd, "highlight_base_name=%d\n", (int) this->pl->highlightBaseName);
   fprintf(fd, "highlight_megabytes=%d\n", (int) this->pl->highlightMegabytes);
   fprintf(fd, "tree_view=%d\n", (int) this->pl->treeView);
   fprintf(fd, "header_margin=%d\n", (int) this->header->margin);
   fprintf(fd, "left_meters=");
   for (int i = 0; i < Header_size(this->header, LEFT_HEADER); i++)
      fprintf(fd, "%s ", Header_readMeterName(this->header, i, LEFT_HEADER));
   fprintf(fd, "\n");
   fprintf(fd, "left_meter_modes=");
   for (int i = 0; i < Header_size(this->header, LEFT_HEADER); i++)
      fprintf(fd, "%d ", Header_readMeterMode(this->header, i, LEFT_HEADER));
   fprintf(fd, "\n");
   fprintf(fd, "right_meters=");
   for (int i = 0; i < Header_size(this->header, RIGHT_HEADER); i++)
      fprintf(fd, "%s ", Header_readMeterName(this->header, i, RIGHT_HEADER));
   fprintf(fd, "\n");
   fprintf(fd, "right_meter_modes=");
   for (int i = 0; i < Header_size(this->header, RIGHT_HEADER); i++)
      fprintf(fd, "%d ", Header_readMeterMode(this->header, i, RIGHT_HEADER));
   fprintf(fd, "\n");
   
   fclose(fd);
   return true;
}
