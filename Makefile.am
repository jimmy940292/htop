
bin_PROGRAMS = htop
htop_SOURCES = debug.h CRT.c CRT.h Hashtable.c Hashtable.h htop.c ListBox.c ListBox.h Object.c Object.h \
Process.c ProcessFilter.c ProcessFilter.h Process.h ProcessList.c ProcessList.h String.c \
String.h TypedVector.c TypedVector.h UsersTable.c UsersTable.h MemoryMonitor.c MemoryMonitor.h \
RichString.c RichString.h Signal.c Signal.h Settings.c Settings.h
dist_man_MANS = htop.1
EXTRA_DIST = $(dist_man_MANS)

AM_CFLAGS = -pedantic -Wall -Werror -std=c99
