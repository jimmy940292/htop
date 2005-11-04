/*
htop - htop.c
(C) 2004,2005 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ProcessList.h"
#include "CRT.h"
#include "ListBox.h"
#include "UsersTable.h"
#include "SignalItem.h"
#include "RichString.h"
#include "Settings.h"
#include "ScreenManager.h"
#include "FunctionBar.h"
#include "ListItem.h"
#include "CategoriesListBox.h"
#include "SignalsListBox.h"

#include "config.h"
#include "debug.h"

#include <unistd.h>
#include <math.h>
#include <sys/param.h>
#include <ctype.h>
#include <stdbool.h>

int usleep(int usec);

//#link m

#define INCSEARCH_MAX 40

/* private property */
char htop_barCharacters[] = "|#*@$%&";

void printVersionFlag() {
   clear();
   printf("htop " VERSION " - (C) 2004,2005 Hisham Muhammad.\n");
   printf("Released under the GNU GPL.\n\n");
   exit(0);
}

void printHelpFlag() {
   clear();
   printf("htop " VERSION " - (C) 2004,2005 Hisham Muhammad.\n");
   printf("Released under the GNU GPL.\n\n");
   printf("-d DELAY    Delay between updates, in tenths of seconds\n\n");
   printf("Press F1 inside htop for online help.\n");
   printf("See the man page for full info.\n\n");
   exit(0);
}

void showHelp() {
   clear();
   attrset(CRT_colors[HELP_BOLD]);
   for (int i = 0; i < LINES; i++) {
      move(i, 0); hline(' ', COLS);
   }
   mvaddstr(0, 0, "htop " VERSION " - (C) 2004 Hisham Muhammad.");
   mvaddstr(1, 0, "Released under the GNU GPL. See man page for more info.");
   attrset(CRT_colors[DEFAULT_COLOR]);
   mvaddstr(3, 0, "CPU usage bar: ");
   #define addattrstr(a,s) attrset(a);addstr(s)
   addattrstr(CRT_colors[BAR_BORDER], "[");
   addattrstr(CRT_colors[CPU_NICE], "low-priority"); addstr("/");
   addattrstr(CRT_colors[CPU_NORMAL], "normal"); addstr("/");
   addattrstr(CRT_colors[CPU_KERNEL], "kernel");
   addattrstr(CRT_colors[BAR_SHADOW], "      used%");
   addattrstr(CRT_colors[BAR_BORDER], "]");
   attrset(CRT_colors[DEFAULT_COLOR]);
   mvaddstr(4, 0, "Memory bar:    ");
   addattrstr(CRT_colors[BAR_BORDER], "[");
   addattrstr(CRT_colors[MEMORY_USED], "used"); addstr("/");
   addattrstr(CRT_colors[MEMORY_BUFFERS], "buffers"); addstr("/");
   addattrstr(CRT_colors[MEMORY_CACHE], "cache");
   addattrstr(CRT_colors[BAR_SHADOW], "         used/total");
   addattrstr(CRT_colors[BAR_BORDER], "]");
   attrset(CRT_colors[DEFAULT_COLOR]);
   mvaddstr(5, 0, "Swap bar:      ");
   addattrstr(CRT_colors[BAR_BORDER], "[");
   addattrstr(CRT_colors[SWAP], "used");
   addattrstr(CRT_colors[BAR_SHADOW], "                       used/total");
   addattrstr(CRT_colors[BAR_BORDER], "]");
   attrset(CRT_colors[DEFAULT_COLOR]);

   attrset(CRT_colors[HELP_BOLD]);
   mvaddstr(7, 0, "Keyboard shortcuts");
   attrset(CRT_colors[DEFAULT_COLOR]);
   mvaddstr(8, 0,  " Arrows - scroll process list        Digits - incremental PID search");
   mvaddstr(9, 0,  " Space - tag process                 / - incremental name search");
   mvaddstr(10, 0, " k - kill process/tagged processes   U - shadow other users");
   mvaddstr(11, 0, " I - invert sort order               t - tree view");
   mvaddstr(12, 0, " P - sort by CPU%                    K - hide kernel threads");
   mvaddstr(13, 0, " M - sort by MEM%                    F - cursor follows process");
   mvaddstr(14, 0, " T - sort by TIME                    Ctrl-L - refresh");
   mvaddstr(15, 0, " [ - decrease priority               ] - increase priority (superuser)");
   mvaddstr(16, 0, " C - configure columns               S - setup");
   mvaddstr(17, 0, " h - shows this help screen          q - quit");
   attrset(CRT_colors[HELP_BOLD]);
   mvaddstr(19,0, "Press any key to return.");
   attrset(CRT_colors[DEFAULT_COLOR]);
   refresh();
   CRT_readKey();
   clear();
}

void showColumnConfig(ProcessList* pl) {

   int i;
   int startSelected = 0;
   int startAvailable = 0;
   int currRow = 0;
   int currCol = 0;
   bool configure = true;
   bool save = false;

   ProcessField avail[LAST_PROCESSFIELD + 1] = { 0 };
   ProcessField select[LAST_PROCESSFIELD + 1] = { 0 };
   ProcessField original[LAST_PROCESSFIELD + 1] = { 0 };
   int countAvail = 0;
   int countSelect = 0;
   int countOriginal = 0;
   
   for(i = 0; i < LAST_PROCESSFIELD && pl->fields[i] != LAST_PROCESSFIELD; i++) {
      select[i] = pl->fields[i];
      original[i] = pl->fields[i];
      countSelect++;
   }
   countOriginal = countSelect;
   select[countSelect] = LAST_PROCESSFIELD;
   original[i] = pl->fields[i];
   for(i = 0; i < LAST_PROCESSFIELD; i++) {
      bool found = false;
      for(int j = 0; j < LAST_PROCESSFIELD && pl->fields[j] != LAST_PROCESSFIELD; j++) 
         if(i == pl->fields[j]) found = true;
      if(!found) {
         avail[countAvail] = i;
         countAvail++;
      }
   }
   avail[countAvail] = LAST_PROCESSFIELD;

   clear();
   mvaddstr(0, 0, "Column configuration");
   attron(CRT_colors[HELP_BOLD]);
   mvaddstr(4, 1, "Selected Columns");
   attroff(CRT_colors[HELP_BOLD]);
   attron(CRT_colors[HELP_BOLD]);
   mvaddstr(4, (COLS / 2) + 1, "Available Columns");
   attroff(CRT_colors[HELP_BOLD]);
   char* functions[5] = { "Move Up", "Move Down", "Move <->", "Apply ", "Cancel" };
   char* keys[5] = { "- ", "+ ", "Enter", "w ", "Esc" };
   int events[5] = { '-', '+', 13, 'w', 27 };
   FunctionBar* fuBar = FunctionBar_new(5, functions, keys, events);
   FunctionBar_draw(fuBar, NULL);
   
   while(configure) {

      for(i = 0; i < LAST_PROCESSFIELD; i++) 
         pl->fields[i] = select[i];
         
      for(i = 0; i < LAST_PROCESSFIELD; i++) {
         int field = select[i + startSelected];
         if(field == LAST_PROCESSFIELD) break;
         if(i == (LINES  - 8)) break;
         mvhline(5 + i, 1, ' ', COLS / 2);
         mvaddstr(5 + i, 1, Process_fieldNames[field]);
      }
      for (; i < LINES - 8; i++)
         mvhline(5 + i, 1, ' ', COLS / 2);

      RichString str = ProcessList_printHeader(pl);
      if (str.len > 0) {
         int attr = CRT_colors[PANEL_HEADER_FOCUS];
         attron(attr);
         RichString_applyAttr(&str, attr);
         move(2, 0);
         hline(' ', 512);
         mvaddchstr(2, 0, str.chstr);
         attroff(attr);
      }

      for(i = 0; i < LAST_PROCESSFIELD; i++) {
         int field = avail[i + startAvailable];
         if(field == LAST_PROCESSFIELD) break;
         if(i == (LINES - 8)) break;
         mvhline(5 + i, (COLS / 2) + 1, ' ', COLS / 2);
         mvaddstr(5 + i, (COLS / 2) + 1, Process_fieldNames[field]);
      }
      for (; i < LINES - 8; i++)
         mvhline(5 + i, (COLS / 2) + 1, ' ', COLS / 2);
      mvchgat(5 + currRow, (currCol) ? (COLS / 2) + 1 : 1, (COLS / 2) - 2,
         A_REVERSE, CRT_colors[PANEL_HIGHLIGHT_FOCUS], NULL);

      refresh();

      int *numEntries = (currCol) ? &countAvail : &countSelect;
      int *notEntries = (currCol) ? &countSelect : &countAvail;
      int *start = (currCol) ? &startAvailable : &startSelected;
      int pos = currRow + *start;
      
      int c = getch();

      mvchgat(5 + currRow, (currCol) ? (COLS / 2) + 1 : 1, (COLS / 2) - 2,
         A_NORMAL, 0, NULL);

      switch(c) {
      case KEY_DOWN:
         if(currRow + *start == *numEntries - 1) break;
         if(currRow < LINES - 9) currRow++;
         else {
            if((*numEntries - *start) > (LINES - 8))
               (*start)++;
         }
         break;

      case KEY_NPAGE:
         // TODO: quick and dirty hack. Better improve.
         for (int i = 0; i < LINES - 9; i++) {
            if(currRow + *start == *numEntries - 1) break;
               if(currRow < LINES - 9) currRow++;
               else {
                  if((*numEntries - *start) > (LINES - 8))
                     (*start)++;
               }
         }
         break;

      case KEY_PPAGE:
         // TODO: quick and dirty hack. Better improve.
         for (int i = 0; i < LINES - 9; i++) {
            if(currRow > 0) currRow--;
            else {
               if(*start > 0)
                  (*start)--;
            }
         }
         break;

      case KEY_UP:
         if(currRow > 0) currRow--;
         else {
            if(*start > 0)
               (*start)--;
         }
         break;

      case KEY_LEFT:
         currCol = 0;
         if(currRow > *notEntries - 1) currRow = *notEntries - 1;
         break;

      case KEY_RIGHT:
         if(countAvail == 0) break;
         currCol = 1;
         if(currRow > *notEntries - 1) currRow = *notEntries - 1;
         break;

      case '}':
      case ']':
      case '+':
      case '.':
      case '=': {
         if(currRow  + *start == *numEntries - 1) break;
         ProcessField *array = (currCol) ? avail : select;
         ProcessField inv = array[pos];
         array[pos] = array[pos + 1];
         array[pos + 1] = inv;
         if(currRow < LINES - 9) currRow++; //From Key Down
         else {
            if((*numEntries - *start) > (LINES - 8))
               (*start)++;
         }
         break;
      }

      case '{':
      case '[':
      case '_':
      case ',':
      case '-': {
         if(currRow + *start == 0) break;
         ProcessField *array = (currCol) ? avail : select;
         ProcessField inv = array[pos];
         array[pos] = array[pos - 1];
         array[pos - 1] = inv;
         if(currRow > 0) currRow--; //From Key up
         else {
            if(*start > 0)
               (*start)--;
         }
         break;
      }

      case 0x0a:
      case 0x0d:
      case KEY_ENTER:
         if(*numEntries == 0) break;
         if(!currCol && *numEntries == 1) break;
         ProcessField *array = (currCol) ? avail : select;
         ProcessField *notarray = (currCol) ? select : avail;
         for(i = *notEntries + 2; i >=1; i--) {
	    notarray[i] = notarray[i-1];
         }
         notarray[0] = array[pos];
         (*notEntries)++;

         for(i = pos; pos < LAST_PROCESSFIELD; i++) {
            if(array[i] == LAST_PROCESSFIELD) break;
            array[i] = array[i + 1];
         }
         (*numEntries)--;
         array[*numEntries] = LAST_PROCESSFIELD;
         if(*start > 0) (*start)--;
         else
            if(pos > *numEntries - 1) currRow--;
	 
         currCol = currCol == 0 ? 1 : 0;
	 currRow = 0;
	 
         if(*numEntries == 0) {
            currCol = 0;
            currRow = 0;
         }
         break;

      case 27:
      case 'q':
         configure = false;
         break;

      case 'w':
         save = true;
         configure = false;
         break;

      default:
         break;
      }
   }

   if(save) {
      for(i = 0; i < LAST_PROCESSFIELD && select[i] != LAST_PROCESSFIELD; i++)
         pl->fields[i] = select[i];
      pl->fields[countSelect] = LAST_PROCESSFIELD;
   }
   else {
      for(i = 0; i < LAST_PROCESSFIELD && original[i] != LAST_PROCESSFIELD; i++)
         pl->fields[i] = original[i];
      pl->fields[countOriginal] = LAST_PROCESSFIELD;
   }
   FunctionBar_delete(fuBar);

   clear();

}

void Setup_run(Settings* settings, int headerHeight) {
   ScreenManager* scr = ScreenManager_new(0, headerHeight, 0, -1, HORIZONTAL, true);
   CategoriesListBox* lbCategories = CategoriesListBox_new(settings, scr);
   ScreenManager_add(scr, (ListBox*) lbCategories, 16);
   CategoriesListBox_makeMetersPage(lbCategories);
   ListBox* lbFocus;
   int ch;
   ScreenManager_run(scr, &lbFocus, &ch);
   ScreenManager_delete(scr);
}

int main(int argc, char** argv) {

   int delay = -1;

   if (argc > 0) {
      if (String_eq(argv[1], "--help")) {
         printHelpFlag();
      } else if (String_eq(argv[1], "--version")) {
         printVersionFlag();
      } else if (String_eq(argv[1], "-d")) {
         if (argc < 2) printHelpFlag();
         sscanf(argv[2], "%d", &delay);
         if (delay < 1) delay = 1;
         if (delay > 100) delay = 100;
      }
   }

   ListBox* lb;
   int quit = 0;
   int refreshTimeout = 0;
   int resetRefreshTimeout = 5;
   bool doRefresh = true;
   Settings* settings;
   
   ListBox* lbk = NULL;

   char incSearchBuffer[INCSEARCH_MAX];
   int incSearchIndex = 0;
   incSearchBuffer[0] = 0;
   bool incSearchMode = false;

   ProcessList* pl = NULL;
   UsersTable* ut = UsersTable_new();

   pl = ProcessList_new(ut);
   
   Header* header = Header_new(pl);
   settings = Settings_new(pl, header);
   int headerHeight = Header_calculateHeight(header);

   // FIXME: move delay code to settings
   if (delay != -1)
      settings->delay = delay;
   
   CRT_init(settings->delay, settings->colorScheme);
   
   lb = ListBox_new(0, headerHeight, COLS, LINES - headerHeight - 2, PROCESS_CLASS, false);
   ListBox_setHeader(lb, ProcessList_printHeader(pl));
   
   char* searchFunctions[3] = {"Next  ", "Exit  ", " Search: "};
   char* searchKeys[3] = {"F3", "Esc", "  "};
   int searchEvents[3] = {KEY_F(3), 27, ERR};
   FunctionBar* searchBar = FunctionBar_new(3, searchFunctions, searchKeys, searchEvents);
   
   char* defaultFunctions[10] = {"Help  ", "Setup ", "Search", "Invert", "Tree  ",
       "SortBy", "Nice -", "Nice +", "Kill  ", "Quit  "};
   FunctionBar* defaultBar = FunctionBar_new(10, defaultFunctions, NULL, NULL);

   ProcessList_scan(pl);
   usleep(75000);
   
   FunctionBar_draw(defaultBar, NULL);
   
   int acc = 0;
   bool follow = false;
 
   struct timeval tv;
   double time = 0.0;
   double oldTime = 0.0;
   bool recalculate;

   while (!quit) {
      gettimeofday(&tv, NULL);
      time = ((double)tv.tv_sec * 10) + ((double)tv.tv_usec / 100000);
      recalculate = (time - oldTime > CRT_delay);
      if (recalculate)
         oldTime = time;
      if (doRefresh) {
         incSearchIndex = 0;
         incSearchBuffer[0] = 0;
         int currPos = ListBox_getSelectedIndex(lb);
         int currPid = 0;
         int currScrollV = lb->scrollV;
         if (follow)
            currPid = ProcessList_get(pl, currPos)->pid;
         if (recalculate)
            ProcessList_scan(pl);
         if (refreshTimeout == 0) {
            ProcessList_sort(pl);
            refreshTimeout = 1;
         }
         ListBox_prune(lb);
         int size = ProcessList_size(pl);
         for (int i = 0; i < size; i++) {
            Process* p = ProcessList_get(pl, i);
            ListBox_set(lb, i, (Object*)p);
            if ((!follow && i == currPos) || (follow && p->pid == currPid)) {
               ListBox_setSelected(lb, i);
               lb->scrollV = currScrollV;
            }
         }
      }
      doRefresh = true;
      
      Header_draw(header);

      ListBox_draw(lb, true);
      int ch = getch();
      if (incSearchMode) {
         doRefresh = false;
         if (ch == ERR) {
            continue;
         } else if (ch == KEY_F(3)) {
            int here = ListBox_getSelectedIndex(lb);
            int size = ProcessList_size(pl);
            int i = here+1;
            while (i != here) {
               if (i == size)
                  i = 0;
               Process* p = ProcessList_get(pl, i);
               if (String_contains_i(p->comm, incSearchBuffer)) {
                  ListBox_setSelected(lb, i);
                  break;
               }
               i++;
            }
            continue;
         } else if (isprint((char)ch) && (incSearchIndex < INCSEARCH_MAX)) {
            incSearchBuffer[incSearchIndex] = ch;
            incSearchIndex++;
            incSearchBuffer[incSearchIndex] = 0;
         } else if ((ch == KEY_BACKSPACE || ch == 127) && (incSearchIndex > 0)) {
            incSearchIndex--;
            incSearchBuffer[incSearchIndex] = 0;
         } else {
            incSearchMode = false;
            incSearchIndex = 0;
            incSearchBuffer[0] = 0;
            FunctionBar_draw(defaultBar, NULL);
            continue;
         }

         bool found = false;
         for (int i = 0; i < ProcessList_size(pl); i++) {
            Process* p = ProcessList_get(pl, i);
            if (String_contains_i(p->comm, incSearchBuffer)) {
               ListBox_setSelected(lb, i);
               found = true;
               break;
            }
         }
         if (found)
            FunctionBar_draw(searchBar, incSearchBuffer);
         else
            FunctionBar_drawAttr(searchBar, incSearchBuffer, CRT_colors[FAILED_SEARCH]);

         continue;
      }
      if (isdigit((char)ch)) {
         int pid = ch-48 + acc;
         for (int i = 0; i < ProcessList_size(pl) && ((Process*) ListBox_getSelected(lb))->pid != pid; i++)
            ListBox_setSelected(lb, i);
         acc = pid * 10;
         if (acc > 100000)
            acc = 0;
         continue;
      } else {
         acc = 0;
      }

      if (ch == KEY_MOUSE) {
         MEVENT mevent;
         int ok = getmouse(&mevent);
         if (ok == OK) {
            if (mevent.y >= lb->y + 1 && mevent.y < LINES - 1) {
               ListBox_setSelected(lb, mevent.y - lb->y + lb->scrollV - 1);
               doRefresh = false;
               refreshTimeout = resetRefreshTimeout;
               follow = true;
               continue;
            } if (mevent.y == LINES - 1) {
               FunctionBar* bar;
               if (incSearchMode) bar = searchBar;
               else bar = defaultBar;
               ch = FunctionBar_synthesizeEvent(bar, mevent.x);
            }

         }
      }

      switch (ch) {
      case ERR:
         refreshTimeout--;
         continue;
      case KEY_RESIZE:
         ListBox_resize(lb, COLS, LINES-headerHeight-1);
         if (incSearchMode)
            FunctionBar_draw(searchBar, incSearchBuffer);
         else
            FunctionBar_draw(defaultBar, NULL);
         break;
      case 'M':
      {
         refreshTimeout = 0;
         pl->sortKey = PERCENT_MEM;
         pl->treeView = false;
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         break;
      }
      case 'T':
      {
         refreshTimeout = 0;
         pl->sortKey = TIME;
         pl->treeView = false;
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         break;
      }
      case 'P':
      {
         refreshTimeout = 0;
         pl->sortKey = PERCENT_CPU;
         pl->treeView = false;
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         break;
      }
      case KEY_F(1):
      case 'h':
      {
         showHelp();
         FunctionBar_draw(defaultBar, NULL);
         refreshTimeout = 0;
         break;
      }
      case '\012': // Enter
      case '\014': // Ctrl+L
      {
         clear();
         FunctionBar_draw(defaultBar, NULL);
         refreshTimeout = 0;
         break;
      }
      case ' ':
      {
         Process* p = (Process*) ListBox_getSelected(lb);
         Process_toggleTag(p);
         ListBox_onKey(lb, KEY_DOWN);
         break;
      }
      case 'S':
      case KEY_F(2):
      {
         Setup_run(settings, headerHeight);
         // TODO: shouldn't need this, colors should be dynamic
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         headerHeight = Header_calculateHeight(header);
         ListBox_move(lb, 0, headerHeight);
         ListBox_resize(lb, COLS, LINES-headerHeight-1);
         FunctionBar_draw(defaultBar, NULL);
         refreshTimeout = 0;
         break;
      }
      case 'F':
      {
         follow = true;
         continue;
      }
      case KEY_F(9):
      case 'k':
      {
         const int lbkWidth = 15;
         if (!lbk) {
            lbk = (ListBox*) SignalsListBox_new(0, headerHeight, lbkWidth-1, LINES - headerHeight - 2);
         }
         SignalsListBox_reset((SignalsListBox*) lbk);

         char* fuFunctions[2] = {"Send  ", "Cancel "};
         char* fuKeys[2] = {"Enter", "Esc"};
         int fuEvents[2] = {13, 27};
         FunctionBar* fuBar = FunctionBar_new(2, fuFunctions, fuKeys, fuEvents);

         ScreenManager* scr = ScreenManager_new(0, headerHeight, 0, -1, HORIZONTAL, false);
         ScreenManager_add(scr, lbk, lbkWidth - 1);
         ScreenManager_add(scr, lb, -1);
         ScreenManager_setFunctionBar(scr, fuBar);
         ListBox* lbFocus;
         int ch;
         ScreenManager_run(scr, &lbFocus, &ch);

         if (lbFocus == lbk && ch == 13) {
            Signal* signal = (Signal*) ListBox_getSelected(lbk);
            if (signal->number != 0) {
               ListBox_setHeader(lbk, RichString_quickString(CRT_colors[PANEL_HEADER_FOCUS], "Sending..."));
               ListBox_draw(lbk, true);
               refresh();
               bool anyTagged = false;
               for (int i = 0; i < ListBox_getSize(lb); i++) {
                  Process* p = (Process*) ListBox_get(lb, i);
                  if (p->tag) {
                     Process_sendSignal(p, signal->number);
                     Process_toggleTag(p);
                     anyTagged = true;
                  }
               }
               if (!anyTagged) {
                  Process* p = (Process*) ListBox_getSelected(lb);
                  Process_sendSignal(p, signal->number);
               }
               napms(500);
            }
         }
         
         FunctionBar_delete(fuBar);
         ScreenManager_delete(scr);
         
         ListBox_move(lb, 0, headerHeight);
         ListBox_resize(lb, COLS, LINES-headerHeight-1);
         FunctionBar_draw(defaultBar, NULL);

         break;
      }
      case KEY_F(10):
      case 'q':
         quit = 1;
         break;
      case '<':
      case ',':
      case KEY_F(18):
      {
         refreshTimeout = 0;
         pl->treeView = false;
         ProcessList_sortKey(pl, -1);
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         break;
      }
      case '>':
      case '.':
      case KEY_F(6):
      {
         refreshTimeout = 0;
         pl->treeView = false;
         ProcessList_sortKey(pl, 1);
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         break;
      }
      case 'I':
      case KEY_F(4):
      {
         refreshTimeout = 0;
         ProcessList_invertSortOrder(pl);
         break;
      }
      case KEY_F(8):
      case '[':
      case '=':
      case '+':
      {
         Process* p = (Process*) ListBox_getSelected(lb);;
         Process_setPriority(p, p->nice + 1);
         doRefresh = false;
         break;
      }
      case KEY_F(7):
      case ']':
      case '-':
      {
         Process* p = (Process*) ListBox_getSelected(lb);;
         Process_setPriority(p, p->nice - 1);
         doRefresh = false;
         break;
      }
      case KEY_F(3):
      case '/':
         FunctionBar_draw(searchBar, incSearchBuffer);
         incSearchMode = true;
         break;
      case 'C':
         showColumnConfig(pl);
         FunctionBar_draw(defaultBar, NULL);
         ListBox_setHeader(lb, ProcessList_printHeader(pl));
         refreshTimeout = 0;
         break;
      case 't':
      case KEY_F(5):
         refreshTimeout = 0;
         pl->treeView = !pl->treeView;
         break;
      case 'H':
         refreshTimeout = 0;
         pl->hideThreads = !pl->hideThreads;
         break;
      case 'U':
         refreshTimeout = 0;
         pl->shadowOtherUsers = !pl->shadowOtherUsers;
         break;
      case 'K':
         refreshTimeout = 0;
         pl->hideKernelThreads = !pl->hideKernelThreads;
         break;
      default:
         doRefresh = false;
         refreshTimeout = resetRefreshTimeout;
         ListBox_onKey(lb, ch);
         break;
      }
      follow = false;
   }
   attron(CRT_colors[RESET_COLOR]);
   mvhline(LINES-1, 0, ' ', COLS);
   attroff(CRT_colors[RESET_COLOR]);
   refresh();
   
   CRT_done();
   Settings_write(settings);
   Header_delete(header);
   ProcessList_delete(pl);
   FunctionBar_delete(searchBar);
   FunctionBar_delete(defaultBar);
   ((Object*)lb)->delete((Object*)lb);
   if (lbk)
      ((Object*)lbk)->delete((Object*)lbk);
   UsersTable_delete(ut);
   Settings_delete(settings);
   debug_done();
   return 0;
}
