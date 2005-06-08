/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "CRT.h"

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include "String.h"

#include "debug.h"

#define WHITE_PAIR 0
#define BLUE_PAIR 1
#define GREEN_PAIR 2
#define RED_PAIR 3
#define BROWN_PAIR 4
#define CYAN_PAIR 5
#define BLACK_PAIR 6
#define BLACK_CYAN_PAIR 7
#define RED_CYAN_PAIR 8
#define BLACK_GREEN_PAIR 9
#define BLACK_WHITE_PAIR 10

#define MIN_UPDATE_SLICE 15

//#link curses

bool CRT_hasColors;

/*{

typedef enum ColorElements_ {
   RESET_COLOR,
   DEFAULT_COLOR,
   FUNCTION_BAR,
   FUNCTION_KEY,
   FAILED_SEARCH,
   PANEL_HEADER_FOCUS,
   PANEL_HEADER_UNFOCUS,
   PANEL_HIGHLIGHT_FOCUS,
   PANEL_HIGHLIGHT_UNFOCUS,
   LARGE_NUMBER,
   METER_TEXT,
   METER_VALUE,
   LED_COLOR,
   UPTIME,
   TASKS_TOTAL,
   TASKS_RUNNING,
   SWAP,
   PROCESS,
   PROCESS_SHADOW,
   PROCESS_TAG,
   PROCESS_MEGABYTES,
   PROCESS_TREE,
   PROCESS_R_STATE,
   PROCESS_BASENAME,
   PROCESS_HIGH_PRIORITY,
   PROCESS_LOW_PRIORITY,
   BAR_BORDER,
   BAR_SHADOW,
   GRAPH_1,
   GRAPH_2,
   GRAPH_3,
   GRAPH_4,
   GRAPH_5,
   GRAPH_6,
   GRAPH_7,
   GRAPH_8,
   GRAPH_9,
   MEMORY_USED,
   MEMORY_BUFFERS,
   MEMORY_CACHE,
   LOAD,
   LOAD_AVERAGE_FIFTEEN,
   LOAD_AVERAGE_FIVE,
   LOAD_AVERAGE_ONE,
   CHECK_BOX,
   CHECK_MARK,
   CHECK_TEXT,
   CLOCK,
   CPU_NICE,
   CPU_NORMAL,
   CPU_KERNEL,
   HELP_BOLD,
   LAST_COLORELEMENT
} ColorElements;

extern int CRT_delay;

extern int CRT_colors[LAST_COLORELEMENT];

}*/

/* private property */
int CRT_delay; 

/* private property */
int CRT_colors[LAST_COLORELEMENT];

void CRT_init() {
   initscr();
   noecho();
   halfdelay(MIN_UPDATE_SLICE);
   nonl();
   intrflush(stdscr, false);
   keypad(stdscr, true);
   curs_set(0);
   if (has_colors()) {
      start_color();
      CRT_hasColors = true;
   } else {
      CRT_hasColors = false;
   }
   char* termType = getenv("TERM");
   if (String_eq(termType, "xterm") || String_eq(termType, "xterm-color") || String_eq(termType, "vt220")) {
      define_key("\033OP", KEY_F(1));
      define_key("\033OQ", KEY_F(2));
      define_key("\033OR", KEY_F(3));
      define_key("\033OS", KEY_F(4));
      define_key("\033[11~", KEY_F(1));
      define_key("\033[12~", KEY_F(2));
      define_key("\033[13~", KEY_F(3));
      define_key("\033[14~", KEY_F(4));
   }
#ifndef DEBUG
   signal(11, CRT_handleSIGSEGV);
#endif
   signal(SIGTERM, CRT_handleSIGTERM);
   use_default_colors();
   init_pair(BLUE_PAIR, COLOR_BLUE, -1);
   init_pair(GREEN_PAIR, COLOR_GREEN, -1);
   init_pair(RED_PAIR, COLOR_RED, -1);
   init_pair(BROWN_PAIR, COLOR_YELLOW, -1);
   init_pair(CYAN_PAIR, COLOR_CYAN, -1);
   init_pair(BLACK_PAIR, COLOR_BLACK, -1);
   init_pair(BLACK_PAIR, COLOR_BLACK, -1);
   init_pair(BLACK_CYAN_PAIR, COLOR_BLACK, COLOR_CYAN);
   init_pair(RED_CYAN_PAIR, COLOR_RED, COLOR_CYAN);
   init_pair(BLACK_GREEN_PAIR, COLOR_BLACK, COLOR_GREEN);
   init_pair(BLACK_WHITE_PAIR, COLOR_BLACK, COLOR_WHITE);
   if (has_colors()) {
      CRT_colors[RESET_COLOR] = A_NORMAL;
      CRT_colors[DEFAULT_COLOR] = A_NORMAL;
      CRT_colors[FUNCTION_BAR] = COLOR_PAIR(BLACK_CYAN_PAIR);
      CRT_colors[FUNCTION_KEY] = A_NORMAL;
      CRT_colors[PANEL_HEADER_FOCUS] = COLOR_PAIR(BLACK_GREEN_PAIR);
      CRT_colors[PANEL_HEADER_UNFOCUS] = COLOR_PAIR(BLACK_GREEN_PAIR);
      CRT_colors[PANEL_HIGHLIGHT_FOCUS] = COLOR_PAIR(BLACK_CYAN_PAIR);
      CRT_colors[PANEL_HIGHLIGHT_UNFOCUS] = COLOR_PAIR(BLACK_WHITE_PAIR);
      CRT_colors[FAILED_SEARCH] = COLOR_PAIR(RED_CYAN_PAIR);
      CRT_colors[UPTIME] = A_BOLD | COLOR_PAIR(CYAN_PAIR);
      CRT_colors[LARGE_NUMBER] = A_BOLD | COLOR_PAIR(RED_PAIR);
      CRT_colors[METER_TEXT] = COLOR_PAIR(CYAN_PAIR);
      CRT_colors[METER_VALUE] = A_BOLD | COLOR_PAIR(CYAN_PAIR);
      CRT_colors[LED_COLOR] = COLOR_PAIR(GREEN_PAIR);
      CRT_colors[TASKS_RUNNING] = A_BOLD | COLOR_PAIR(GREEN_PAIR);
      CRT_colors[PROCESS] = A_NORMAL;
      CRT_colors[PROCESS_SHADOW] = A_BOLD | COLOR_PAIR(BLACK_PAIR);
      CRT_colors[PROCESS_TAG] = A_BOLD | COLOR_PAIR(BROWN_PAIR);
      CRT_colors[PROCESS_MEGABYTES] = COLOR_PAIR(CYAN_PAIR);
      CRT_colors[PROCESS_BASENAME] = A_BOLD | COLOR_PAIR(CYAN_PAIR);
      CRT_colors[PROCESS_TREE] = COLOR_PAIR(CYAN_PAIR);
      CRT_colors[PROCESS_R_STATE] = COLOR_PAIR(GREEN_PAIR);
      CRT_colors[PROCESS_HIGH_PRIORITY] = COLOR_PAIR(RED_PAIR);
      CRT_colors[PROCESS_LOW_PRIORITY] = COLOR_PAIR(RED_PAIR);
      CRT_colors[BAR_BORDER] = A_BOLD;
      CRT_colors[BAR_SHADOW] = A_BOLD | COLOR_PAIR(BLACK_PAIR);
      CRT_colors[SWAP] = COLOR_PAIR(RED_PAIR);
      CRT_colors[GRAPH_1] = A_BOLD | COLOR_PAIR(RED_PAIR);
      CRT_colors[GRAPH_2] = COLOR_PAIR(RED_PAIR);
      CRT_colors[GRAPH_3] = A_BOLD | COLOR_PAIR(BROWN_PAIR);
      CRT_colors[GRAPH_4] = A_BOLD | COLOR_PAIR(GREEN_PAIR);
      CRT_colors[GRAPH_5] = COLOR_PAIR(GREEN_PAIR);
      CRT_colors[GRAPH_6] = COLOR_PAIR(CYAN_PAIR);
      CRT_colors[GRAPH_7] = A_BOLD | COLOR_PAIR(BLUE_PAIR);
      CRT_colors[GRAPH_8] = COLOR_PAIR(BLUE_PAIR);
      CRT_colors[GRAPH_9] = A_BOLD | COLOR_PAIR(BLACK_PAIR);
      CRT_colors[MEMORY_USED] = COLOR_PAIR(GREEN_PAIR);
      CRT_colors[MEMORY_BUFFERS] = COLOR_PAIR(BLUE_PAIR);
      CRT_colors[MEMORY_CACHE] = COLOR_PAIR(BROWN_PAIR);
      CRT_colors[LOAD_AVERAGE_FIFTEEN] = A_BOLD | COLOR_PAIR(BLACK_PAIR);
      CRT_colors[LOAD_AVERAGE_FIVE] = A_NORMAL;
      CRT_colors[LOAD_AVERAGE_ONE] = A_BOLD;
      CRT_colors[LOAD] = A_BOLD;
      CRT_colors[HELP_BOLD] = A_BOLD | COLOR_PAIR(CYAN_PAIR);
      CRT_colors[CPU_NICE] = COLOR_PAIR(BLUE_PAIR);
      CRT_colors[CPU_NORMAL] = COLOR_PAIR(GREEN_PAIR);
      CRT_colors[CPU_KERNEL] = COLOR_PAIR(RED_PAIR);
      CRT_colors[CLOCK] = A_BOLD;
      CRT_colors[CHECK_BOX] = COLOR_PAIR(CYAN_PAIR);
      CRT_colors[CHECK_MARK] = A_BOLD;
      CRT_colors[CHECK_TEXT] = A_NORMAL;
   } else {
      CRT_colors[RESET_COLOR] = A_NORMAL;
      CRT_colors[DEFAULT_COLOR] = A_NORMAL;
      CRT_colors[FUNCTION_BAR] = A_REVERSE;
      CRT_colors[FUNCTION_KEY] = A_NORMAL;
      CRT_colors[PANEL_HEADER_FOCUS] = A_REVERSE;
      CRT_colors[PANEL_HEADER_UNFOCUS] = A_REVERSE;
      CRT_colors[PANEL_HIGHLIGHT_FOCUS] = A_REVERSE | A_BOLD;
      CRT_colors[PANEL_HIGHLIGHT_UNFOCUS] = A_REVERSE;
      CRT_colors[FAILED_SEARCH] = A_REVERSE | A_BOLD;
      CRT_colors[UPTIME] = A_BOLD;
      CRT_colors[LARGE_NUMBER] = A_BOLD;
      CRT_colors[METER_TEXT] = A_NORMAL;
      CRT_colors[METER_VALUE] = A_BOLD;
      CRT_colors[LED_COLOR] = A_NORMAL;
      CRT_colors[TASKS_RUNNING] = A_BOLD;
      CRT_colors[PROCESS] = A_NORMAL;
      CRT_colors[PROCESS_SHADOW] = A_DIM;
      CRT_colors[PROCESS_TAG] = A_BOLD;
      CRT_colors[PROCESS_MEGABYTES] = A_BOLD;
      CRT_colors[PROCESS_BASENAME] = A_BOLD;
      CRT_colors[PROCESS_TREE] = A_BOLD;
      CRT_colors[PROCESS_R_STATE] = A_BOLD;
      CRT_colors[PROCESS_HIGH_PRIORITY] = A_BOLD;
      CRT_colors[PROCESS_LOW_PRIORITY] = A_DIM;
      CRT_colors[BAR_BORDER] = A_BOLD;
      CRT_colors[BAR_SHADOW] = A_DIM;
      CRT_colors[SWAP] = A_BOLD;
      CRT_colors[GRAPH_1] = A_BOLD;
      CRT_colors[GRAPH_2] = A_BOLD;
      CRT_colors[GRAPH_3] = A_BOLD;
      CRT_colors[GRAPH_4] = A_NORMAL;
      CRT_colors[GRAPH_5] = A_NORMAL;
      CRT_colors[GRAPH_6] = A_NORMAL;
      CRT_colors[GRAPH_7] = A_DIM;
      CRT_colors[GRAPH_8] = A_DIM;
      CRT_colors[GRAPH_9] = A_DIM;
      CRT_colors[MEMORY_USED] = A_BOLD;
      CRT_colors[MEMORY_BUFFERS] = A_NORMAL;
      CRT_colors[MEMORY_CACHE] = A_NORMAL;
      CRT_colors[LOAD_AVERAGE_FIFTEEN] = A_DIM;
      CRT_colors[LOAD_AVERAGE_FIVE] = A_NORMAL;
      CRT_colors[LOAD_AVERAGE_ONE] = A_BOLD;
      CRT_colors[LOAD] = A_BOLD;
      CRT_colors[HELP_BOLD] = A_BOLD;
      CRT_colors[CPU_NICE] = A_NORMAL;
      CRT_colors[CPU_NORMAL] = A_BOLD;
      CRT_colors[CPU_KERNEL] = A_BOLD;
      CRT_colors[CLOCK] = A_BOLD;
      CRT_colors[CHECK_BOX] = A_BOLD;
      CRT_colors[CHECK_MARK] = A_NORMAL;
      CRT_colors[CHECK_TEXT] = A_NORMAL;
   }

   mousemask(BUTTON1_PRESSED, NULL);
}

void CRT_done() {
   curs_set(1);
   endwin();
}

int CRT_readKey() {
   nocbreak();
   cbreak();
   int ret = getch();
   halfdelay(CRT_delay);
   return ret;
}

void CRT_handleSIGSEGV(int signal) {
   CRT_done();
   fprintf(stderr, "Aborted. Please report bug at http://htop.sf.net");
   exit(1);
}

void CRT_handleSIGTERM(int signal) {
   CRT_done();
   exit(0);
}
