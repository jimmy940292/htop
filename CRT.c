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

#include "debug.h"

#define WHITE_PAIR 0
#define BLUE_PAIR 1
#define GREEN_PAIR 2
#define RED_PAIR 3
#define BROWN_PAIR 4
#define CYAN_PAIR 5
#define BLACK_PAIR 6

#define HEADER_PAIR 7
#define INVERSE_PAIR 8
#define FUNCTIONBAR_PAIR 9

#define MIN_UPDATE_SLICE 15

//#link curses

bool CRT_hasColors;

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
//   signal(11, CRT_handleSIGSEGV);
   signal(SIGTERM, CRT_handleSIGTERM);
   use_default_colors();
   init_pair(BLUE_PAIR, COLOR_BLUE, -1);
   init_pair(GREEN_PAIR, COLOR_GREEN, -1);
   init_pair(RED_PAIR, COLOR_RED, -1);
   init_pair(BROWN_PAIR, COLOR_YELLOW, -1);
   init_pair(CYAN_PAIR, COLOR_CYAN, -1);
   init_pair(HEADER_PAIR, COLOR_BLACK, COLOR_GREEN);
   init_pair(BLACK_PAIR, COLOR_BLACK, -1);
   init_pair(INVERSE_PAIR, COLOR_WHITE, COLOR_BLACK);
   init_pair(FUNCTIONBAR_PAIR, COLOR_BLACK, COLOR_CYAN);
}

void CRT_done() {
   curs_set(1);
   endwin();
}

int CRT_readKey() {
   nocbreak();
   cbreak();
   int ret = getch();
   halfdelay(MIN_UPDATE_SLICE);
   return ret;
}

void CRT_handleSIGSEGV(int signal) {
   CRT_done();
   exit(1);
}

void CRT_handleSIGTERM(int signal) {
   CRT_done();
   exit(0);
}
