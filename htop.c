/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ProcessList.h"
#include "CRT.h"
#include "ListBox.h"
#include "UsersTable.h"
#include "Signal.h"
#include "RichString.h"
#include "ProcessFilter.h"
#include "Settings.h"

#include "debug.h"

#include <unistd.h>
#include <math.h>
#include <sys/param.h>
#include <ctype.h>
#include <stdbool.h>

//#link m

#define NICE_CPU_COLOR BLUE_PAIR
#define USER_CPU_COLOR GREEN_PAIR
#define SYSTEM_CPU_COLOR RED_PAIR

#define USED_MEMORY_COLOR GREEN_PAIR
#define BUFFERS_MEMORY_COLOR BLUE_PAIR
#define CACHED_MEMORY_COLOR BROWN_PAIR

#define USED_SWAP_COLOR RED_PAIR

#define HEADER_COLOR INV_PAIR

#define INCSEARCH_MAX 40

typedef enum FunctionBarModes_ {
   DEFAULT,
   SEARCH,
   NONE
} FunctionBarModes;

char htop_barCharacters[] = "|#*@$%&";

void showHelp() {
   clear();
   mvaddstr(0, 0, "htop - (C) 2004 Hisham Muhammad.");
   mvaddstr(1, 0, "Released under the GNU GPL.");
   mvaddstr(3, 0, "CPU usage bar: [");
   attron(COLOR_PAIR(BLUE_PAIR));
   addstr("low-priority");
   attroff(COLOR_PAIR(BLUE_PAIR));
   addstr("/");
   attron(COLOR_PAIR(GREEN_PAIR));
   addstr("normal");
   attroff(COLOR_PAIR(GREEN_PAIR));
   addstr("/");
   attron(COLOR_PAIR(RED_PAIR));
   addstr("kernel");
   attroff(COLOR_PAIR(RED_PAIR));
   attron(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("      used%");
   attroff(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("]");
   mvaddstr(4, 0, "Memory bar:    [");
   attron(COLOR_PAIR(GREEN_PAIR));
   addstr("used");
   attroff(COLOR_PAIR(GREEN_PAIR));
   addstr("/");
   attron(COLOR_PAIR(BLUE_PAIR));
   addstr("buffers");
   attroff(COLOR_PAIR(BLUE_PAIR));
   addstr("/");
   attron(COLOR_PAIR(BROWN_PAIR));
   addstr("cache");
   attroff(COLOR_PAIR(BROWN_PAIR));
   attron(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("         used/total");
   attroff(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("]");
   mvaddstr(5, 0, "Swap bar:      [");
   attron(COLOR_PAIR(RED_PAIR));
   addstr("used");
   attroff(COLOR_PAIR(RED_PAIR));
   attron(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("                       used/total");
   attroff(A_BOLD | COLOR_PAIR(BLACK_PAIR));
   addstr("]");

   mvaddstr(7, 0, "Summary of commands -- see the man page for full info");
   mvaddstr(8, 0, " arrow keys - scroll process list");
   mvaddstr(9, 0, " / - incremental process search: type process name");
   mvaddstr(10, 0, " [ - decrease process priority");
   mvaddstr(11, 0, " ] - increase process priority (superuser only)");
   mvaddstr(12, 0, " k - kill process(es)");
   mvaddstr(13,0, " P - sort by CPU%");
   mvaddstr(14,0, " M - sort by MEM%");
   mvaddstr(15,0, " C - configure columns");
   mvaddstr(16,0, " h - shows this help screen");
   mvaddstr(17,0, " q - quit");
   mvaddstr(19,0, "Press any key to return.");
   refresh();
   CRT_readKey();
   clear();
}

void drawFunctionStrip(int size, char** functions, char** keys) {
   attron(COLOR_PAIR(FUNCTIONBAR_PAIR));
   mvhline(LINES-1, 0, ' ', COLS);
   for (int i = 0; i < size; i++) {
      mvaddstr(LINES-1, i*8+2, functions[i]);
   }
   attroff(COLOR_PAIR(FUNCTIONBAR_PAIR));
   attron(COLOR_PAIR(WHITE_PAIR));
   for (int i = 0; i < size; i++) {
      mvaddstr(LINES-1, i*8, keys[i]);
   }
   attroff(COLOR_PAIR(WHITE_PAIR));
}


void showColumnConfig(ProcessFilter* pf) {
   int i;
   int startSelected = 0;
   int startAvailable = 0;
   int currRow = 0;
   int currCol = 0;
   bool configure = true;
   bool save = false;

   ProcessField avail[LAST + 1] = { 0 };
   ProcessField select[LAST + 1] = { 0 };
   ProcessField original[LAST + 1] = { 0 };
   int countAvail = 0;
   int countSelect = 0;
   
   for(i = 0; i < LAST && pf->fields[i] != LAST; i++) {
      select[i] = pf->fields[i];
      original[i] = pf->fields[i];
      countSelect++;
   }
   select[countSelect] = LAST;
   original[i] = pf->fields[i];
   for(i = 0; i < LAST; i++) {
      bool found = false;
      for(int j = 0; j < LAST && pf->fields[j] != LAST; j++) 
         if(i == pf->fields[j]) found = true;
      if(!found) {
         avail[countAvail] = i;
         countAvail++;
      }
   }
   avail[countAvail] = LAST;
   
   while(configure) {
      clear();
      mvaddstr(0, 0, "htop - (C) 2004 Hisham Muhammad.");
      attron(COLOR_PAIR(BLUE_PAIR));
      mvaddstr(4, 1, "Selected Columns");
      attroff(COLOR_PAIR(BLUE_PAIR));
      attron(COLOR_PAIR(GREEN_PAIR));
      mvaddstr(4, (COLS / 2) + 1, "Available Columns");
      attroff(COLOR_PAIR(GREEN_PAIR));
      char* functions[5] = { "MoveUp", "MoveDn", "Mv <->", "Apply ", "Cancel" };
      char* keys[5] = { "- ", "+ ", "CR", "w ", "q " };
      drawFunctionStrip(5, functions, keys);

      for(i = 0; i < LAST; i++) 
         pf->fields[i] = select[i];
      pf->fields[LAST] = LAST;
         
      for(i = 0; i < LAST; i++) {
         int field = select[i + startSelected];
         if(field == LAST) break;
         if(i == (LINES  - 8)) break;
         mvaddstr(5 + i, 1, processFieldNames[field]);
      }

      RichString str = RichString_new();
      ProcessFilter_display((Object*)pf, &str);
      if (str.len > 0) {
         attron(COLOR_PAIR(HEADER_PAIR));
         RichString_applyAttr(&str, COLOR_PAIR(HEADER_PAIR));
         move(2, 0);
         hline(' ', 512);
         mvaddchstr(2, 0, str.chstr);
         attroff(COLOR_PAIR(HEADER_PAIR));
      }

      for(i = 0; i < LAST; i++) {
         int field = avail[i + startAvailable];
         if(field == LAST) break;
         if(i == (LINES - 8)) break;
         mvaddstr(5 + i, (COLS / 2) + 1, processFieldNames[field]);
      }
      mvchgat(5 + currRow, (currCol) ? (COLS / 2) + 1 : 1, (COLS / 2) - 2,
         A_REVERSE, INVERSE_PAIR, NULL);

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

      case '+':
      case '=': {
         if(currRow  + *start == *numEntries - 1) break;
         ProcessField *array = (currCol) ? avail : select;
         ProcessField swap = array[pos];
         array[pos] = array[pos + 1];
         array[pos + 1] = swap;
         if(currRow < LINES - 9) currRow++; //From Key Down
         else {
            if((*numEntries - *start) > (LINES - 8))
               (*start)++;
         }
         break;
      }

      case '_':
      case '-': {
         if(currRow + *start == 0) break;
         ProcessField *array = (currCol) ? avail : select;
         ProcessField swap = array[pos];
         array[pos] = array[pos - 1];
         array[pos - 1] = swap;
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
         notarray[*notEntries] = array[pos];
         notarray[*notEntries + 1] = LAST;
         (*notEntries)++;

         for(i = pos; pos < LAST; i++) {
            if(array[i] == LAST) break;
            array[i] = array[i + 1];
         }
         (*numEntries)--;
         array[*numEntries] = LAST;
         if(*start > 0) (*start)--;
         else 
            if(pos > *numEntries - 1) currRow--;
         if(*numEntries == 0) {
            currCol = 0;
            currRow = 0;
         }
         break;

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
      for(i = 0; i < LAST && select[i] != LAST; i++)
         pf->fields[i] = select[i];
      pf->fields[countSelect] = LAST;
   }
   else {
      for(i = 0; i < LAST && original[i] != LAST; i++)
         pf->fields[i] = original[i];
      pf->fields[countSelect] = LAST;
   }

   clear();
}

void drawUptime(int x, int y) {
   double uptime;
   FILE* fd = fopen(PROCDIR "/uptime", "r");
   fscanf(fd, "%lf", &uptime);
   fclose(fd);
   int totalseconds = (int) ceil(uptime);
   int seconds = totalseconds % 60;
   int minutes = (totalseconds-seconds) % 3600 / 60;
   int hours = (totalseconds-seconds-(minutes*60)) % 86400 / 3600;
   int days = (totalseconds-seconds-(minutes*60)-(hours*3600)) / 86400;
   char buf[20];
   snprintf(buf, 19, "%3d %02d:%02d:%02d", days, hours, minutes, seconds);
   // TODO: actually display the uptime
}

void drawBar(int x, int y, int w, char* displayValue, int items, ...) {
   assert(displayValue != NULL);
   attron(A_BOLD);
   mvaddch(y, x, '[');
   mvaddch(y, x + w, ']');
   attroff(A_BOLD);
   w--;
   x++;
   char bar[w];
   int attrs[10];
   int blockSizes[10];
   for (int i = 0; i < w; i++)
      bar[i] = ' ';
   sprintf(bar + (w-strlen(displayValue)), "%s", displayValue);
   va_list ap;
   va_start(ap, items);
   double total = 0.0;
   int offset = 0;
   for (int i = 0; i < items; i++) {
      attrs[i] = va_arg(ap, int);
      double value = va_arg(ap, double);
      blockSizes[i] = ceil(value * w);
      int nextOffset = offset + blockSizes[i];
      // Control against invalid values
      nextOffset = MAX(nextOffset, 0);
      nextOffset = MIN(nextOffset, w);
      for (int j = offset; j < nextOffset; j++)
         if (bar[j] == ' ') {
            if (CRT_hasColors) {
               bar[j] = '|';
            } else {
               bar[j] = htop_barCharacters[i];
            }
         }
      offset = nextOffset;
      total += value;
   }
   va_end(ap);
   offset = 0;
   for (int i = 0; i < items; i++) {
      attron(attrs[i]);
      mvaddnstr(y, x + offset, bar + offset, blockSizes[i]);
      attroff(attrs[i]);
      offset += blockSizes[i];
   }
   attron(A_BOLD);
   if (offset < w) {
      attron(COLOR_PAIR(BLACK_PAIR));
      mvaddnstr(y, x + offset, bar + offset, w - offset);
      attroff(COLOR_PAIR(BLACK_PAIR));
   }
   attroff(A_BOLD);
   move(y, x + w + 1);
   clrtoeol();
}

void drawCPUBar(ProcessList* pl, int x, int y, int w) {
   double niceCPU = pl->nicePeriod / (double)pl->totalPeriod;
   double userCPU = pl->userPeriod / (double)pl->totalPeriod;
   double systemCPU = pl->systemPeriod / (double)pl->totalPeriod;
   char total[8];
   snprintf(total, 7, "%5.1f%%", (niceCPU+userCPU+systemCPU)*100.0 );
   drawBar(x+3, y, w, total, 3,
           COLOR_PAIR(NICE_CPU_COLOR), niceCPU,
           COLOR_PAIR(USER_CPU_COLOR), userCPU,
           COLOR_PAIR(SYSTEM_CPU_COLOR), systemCPU);
   mvaddstr(y, x, "CPU");
}

void drawMemoryBar(ProcessList* pl, int x, int y, int w) {
   double totalMem = (double)pl->totalMem;
   long int usedMem = pl->usedMem, buffersMem = pl->buffersMem, cachedMem = pl->cachedMem;
   usedMem -= buffersMem + cachedMem;
   char total[15];
   snprintf(total, 14, "%ld/%ldMB", usedMem / 1024, pl->totalMem / 1024);
   drawBar(x+3, y, w, total, 3,
           COLOR_PAIR(USED_MEMORY_COLOR), usedMem / totalMem,
           COLOR_PAIR(BUFFERS_MEMORY_COLOR), buffersMem / totalMem,
           COLOR_PAIR(CACHED_MEMORY_COLOR), cachedMem / totalMem);
   mvaddstr(y, x, "Mem");
}

void drawSwapBar(ProcessList* pl, int x, int y, int w) {
   double totalSwap = (double)pl->totalSwap;
   long int usedSwap = pl->usedSwap;
   char total[15];
   snprintf(total, 14, "%ld/%ldMB", usedSwap / 1024 / 1024, pl->totalSwap / 1024);
   drawBar(x+3, y, w, total, 1,
           COLOR_PAIR(USED_SWAP_COLOR), usedSwap / totalSwap);
   mvaddstr(y, x, "Swp");
}
void drawFunctionBar(FunctionBarModes mode, char* buffer) {
   switch (mode) {
   case SEARCH:
      attron(COLOR_PAIR(FUNCTIONBAR_PAIR));
      mvhline(LINES-1, 0, ' ', COLS);
      mvaddstr(LINES-1, 0, "Search:");
      mvaddstr(LINES-1, 8, buffer);
      attroff(COLOR_PAIR(FUNCTIONBAR_PAIR));
      break;
   case DEFAULT:
   {
      char* functions[] = {
         "Help  ", "Follow", "Search", "Invert",
         "Sort<-", "Sort->", "Nice -", "Nice +",
         "Kill  ", "Quit  "
      };
      char* keys[] = {
         " 1", " 2", " 3", " 4",
         " 5", " 6", " 7", " 8",
         " 9", "10"
      };
      drawFunctionStrip(10, functions, keys);
      break;
   }
   default:
   {
   }
   }
}

int main(int argc, char** argv) {

   ListBox* lb;
   int quit = 0;
   int refreshTimeout = 0;
   int resetRefreshTimeout = 5;
   bool doRefresh = true;
   ProcessFilter* pf;
   Settings* settings;

   Signal** signals = NULL;
   ListBox* lbk = NULL;

   char incSearchBuffer[INCSEARCH_MAX];
   int incSearchIndex = 0;
   incSearchBuffer[0] = 0;
   bool incSearchMode = false;

   ProcessList* pl = NULL;
   UsersTable* ut = UsersTable_new();
   pf = ProcessFilter_new();
   settings = Settings_new(pf);
   CRT_init();
   pl = ProcessList_new(pf, ut);
   ProcessList_scan(pl);

   lb = ListBox_new(0, 5, COLS, LINES - 6, PROCESS_CLASS);
   RichString headerString = RichString_new();
   ((Object*)pf)->display((Object*)pf, &headerString);

   ListBox_setHeader(lb, headerString);

   drawFunctionBar(DEFAULT, NULL);

   int acc = 0;
   bool follow = false;
   while (!quit) {
      if (doRefresh) {
         incSearchIndex = 0;
         incSearchBuffer[0] = 0;
         int currPos = ListBox_getSelectedIndex(lb);
         int currPid = 0;
         int currScrollV = lb->scrollV;
         if (follow)
            currPid = ProcessList_get(pl, currPos)->pid;
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
      
      drawCPUBar(pl, 2, 1, COLS/2-4);
      drawMemoryBar(pl, 2, 2, COLS/2-4);
      drawSwapBar(pl, 2, 3, COLS/2-4);
      // Coming soon! :-)
      // drawUptime(COLS/2+15, 0);

      ListBox_draw(lb);
      int ch = getch();
      if (incSearchMode) {
         doRefresh = false;
         if (isalnum(ch) && (incSearchIndex < INCSEARCH_MAX)) {
            incSearchBuffer[incSearchIndex] = ch;
            incSearchIndex++;
            incSearchBuffer[incSearchIndex] = 0;
            drawFunctionBar(SEARCH, incSearchBuffer);
            for (int i = 0; i < ProcessList_size(pl); i++) {
               Process* p = ProcessList_get(pl, i);
               if (String_contains_i(p->comm, incSearchBuffer)) {
                  ListBox_setSelected(lb, i);
                  break;
               }
            }
         } else if ((ch == KEY_BACKSPACE) && (incSearchIndex > 0)) {
            incSearchIndex--;
            incSearchBuffer[incSearchIndex] = 0;
            drawFunctionBar(SEARCH, incSearchBuffer);
          } else if (ch != ERR) {
            incSearchMode = false;
            incSearchIndex = 0;
            incSearchBuffer[0] = 0;
            drawFunctionBar(DEFAULT, NULL);
         }
         continue;
      }
      if (isdigit(ch)) {
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
      switch (ch) {
      case ERR:
         refreshTimeout--;
         continue;
      case KEY_RESIZE:
         ListBox_resize(lb, COLS, LINES-6);
         drawFunctionBar(incSearchMode?SEARCH:DEFAULT, incSearchBuffer);
         break;
      case 'M':
      {
         refreshTimeout = 0;
         pf->sortKey = PERCENT_MEM;
         headerString = RichString_new();
         ProcessFilter_display((Object*)pf, &headerString);
         ListBox_setHeader(lb, headerString);
         break;
      }
      case 'P':
      {
         refreshTimeout = 0;
         pf->sortKey = PERCENT_CPU;
         headerString = RichString_new();
         ProcessFilter_display((Object*)pf, &headerString);
         ListBox_setHeader(lb, headerString);
         break;
      }
      case KEY_F(1):
      case 'h':
      {
         showHelp();
         drawFunctionBar(DEFAULT, NULL);
         refreshTimeout = 0;
         break;
      }
      case '\014': // Ctrl+L
      {
         clear();
         drawFunctionBar(DEFAULT, NULL);
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
      case KEY_F(2):
      {
         follow = true;
         continue;
      }
      case KEY_F(9):
      case 'k':
      {
         const int lbkWidth = 15;
         int acc = 0;
         ListBox_resize(lb, COLS-lbkWidth, LINES-6);
         ListBox_move(lb, lbkWidth, 5);
         mvvline(5, lbkWidth-1, ' ', LINES-6);
         int sigCount = Signal_getSignalCount();
         if (!signals) {
            signals = Signal_getSignalTable();
            lbk = ListBox_new(0, 5, lbkWidth-1, LINES - 6, SIGNAL_CLASS);
            for(int i = 0; i < sigCount; i++)
               ListBox_set(lbk, i, (Object*) signals[i]);
         }
         RichString_write(&headerString, A_NORMAL, "Send signal:");
         ListBox_setHeader(lbk, headerString);
         ListBox_setSelected(lbk, 16); // 16th item is SIGTERM
         drawFunctionBar(NONE, NULL);
         lbk->needsRedraw = true;
         bool quitKill = false;
         while (!quitKill) {
            ListBox_draw(lb);
            ListBox_draw(lbk);
            int chk = getch();
            if (isdigit(chk)) {
               int signal = chk-48 + acc;
               for (int i = 0; i < sigCount; i++)
                  if (((Signal*) ListBox_get(lbk, i))->number == signal) {
                     ListBox_setSelected(lbk, i);
                     break;
                  }
               acc = signal * 10;
               if (acc > 100)
                  acc = 0;
            } else {
               acc = 0;
            }
            switch (chk) {
            case ERR:
               break;
            case KEY_RESIZE:
               ListBox_resize(lb, COLS-lbkWidth, LINES-6);
               ListBox_resize(lbk, lbkWidth, LINES-6);
               drawFunctionBar(NONE, NULL);
               break;
            case 'q':
            case 27:
               quitKill = true;
               break;
            case 13:
            {
               Signal* signal = (Signal*) ListBox_getSelected(lbk);
               if (signal->number != 0) {
                  RichString_write(&headerString, A_NORMAL, "Sending...");
                  ListBox_setHeader(lbk, headerString);
                  ListBox_draw(lbk);
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
               quitKill = true;
               break;
            }
            default:
               ListBox_onKey(lbk, chk);
               break;
            }
         }
         refreshTimeout = 0;
         ListBox_resize(lb, COLS, LINES-6);
         ListBox_move(lb, 0, 5);
         drawFunctionBar(DEFAULT, NULL);
         break;
      }
      case KEY_F(10):
      case 'q':
         quit = 1;
         break;
      case KEY_F(5):
      {
         refreshTimeout = 0;
         ProcessFilter_sortKey(pf, -1);
         headerString = RichString_new();
         ProcessFilter_display((Object*)pf, &headerString);
         ListBox_setHeader(lb, headerString);
         break;
      }
      case KEY_F(6):
      {
         refreshTimeout = 0;
         ProcessFilter_sortKey(pf, 1);
         headerString = RichString_new();
         ProcessFilter_display((Object*)pf, &headerString);
         ListBox_setHeader(lb, headerString);
         break;
      }
      case KEY_F(4):
      {
         refreshTimeout = 0;
         ProcessFilter_invertSortOrder(pf);
         break;
      }
      case KEY_F(8):
      case '[':
      {
         Process* p = (Process*) ListBox_getSelected(lb);;
         Process_setPriority(p, p->nice + 1);
         doRefresh = false;
         break;
      }
      case KEY_F(7):
      case ']':
      {
         Process* p = (Process*) ListBox_getSelected(lb);;
         Process_setPriority(p, p->nice - 1);
         doRefresh = false;
         break;
      }
      case KEY_F(3):
      case '/':
         drawFunctionBar(SEARCH, incSearchBuffer);
         incSearchMode = true;
         break;
      case 'C':
         showColumnConfig(pf);
         drawFunctionBar(DEFAULT, NULL);
         headerString = RichString_new();
         ProcessFilter_display((Object*)pf, &headerString);
         ListBox_setHeader(lb, headerString);
         refreshTimeout = 0;
         break;
         
      default:
         doRefresh = false;
         refreshTimeout = resetRefreshTimeout;
         ListBox_onKey(lb, ch);
         break;
      }
      follow = false;
   }
   CRT_done();
   Settings_write(settings);
   if (lbk) {
      ListBox_delete(lbk);
      // TODO: delete signals
   }
   ProcessList_delete(pl);
   ListBox_delete(lb);
   UsersTable_delete(ut);
   ProcessFilter_delete(pf);
   Settings_delete(settings);
   /*
   for (int i = 0; i < SIGNAL_COUNT; i++) {
      Signal_delete((Object*)signals[i]);
   }
   free(signals);
   */
   debug_done();
   return 0;
}
