/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"
#include "ProcessFilter.h"
#include <stdlib.h>
#include <string.h>

#include "debug.h"

/*{
typedef enum ProcessField_ {
   PID, COMM, STATE, PPID, PGRP, SESSION, TTY_NR, TPGID, FLAGS, MINFLT, CMINFLT, MAJFLT, CMAJFLT, UTIME,
   STIME, CUTIME, CSTIME, PRIORITY, NICE, ITREALVALUE, STARTTIME, VSIZE, RSS, RLIM, STARTCODE, ENDCODE,
   STARTSTACK, KSTKESP, KSTKEIP, SIGNAL, BLOCKED, SIGIGNORE, SIGCATCH, WCHAN, NSWAP, CNSWAP, EXIT_SIGNAL,
   PROCESSOR, M_SIZE, M_RESIDENT, M_SHARE, M_TRS, M_DRS, M_LRS, M_DT, ST_UID, PERCENT_CPU, PERCENT_MEM, USER, LAST
} ProcessField;

typedef struct ProcessFilter_ {
   Object super;

   ProcessField* fields;
   int width;
   ProcessField sortKey;
   int direction;
} ProcessFilter;

extern char* processFieldNames[];
}*/

/* private property */
char* PROCESSFILTER_CLASS = "ProcessFilter";

/* private property */
char *processFieldNames[] = { "PID", "COMM", "STATE", "PPID", "PGRP", "SESSION", "TTY_NR", "TPGID", "FLAGS", "MINFLT", "CMINFLT", "MAJFLT", "CMAJFLT", "UTIME", "STIME", "CUTIME", "CSTIME", "PRIORITY", "NICE", "ITREALVALUE", "STARTTIME", "VSIZE", "RSS", "RLIM", "STARTCODE", "ENDCODE", "STARTSTACK", "KSTKESP", "KSTKEIP", "SIGNAL", "BLOCKED", "SIGIGNORE", "SIGCATCH", "WCHAN", "NSWAP", "CNSWAP", "EXIT_SIGNAL",  "PROCESSOR", "M_SIZE", "M_RESIDENT", "M_SHARE", "M_TRS", "M_DRS", "M_LRS", "M_DT", "ST_UID", "PERCENT_CPU", "PERCENT_MEM", "USER", "LAST"};

/* private property */
ProcessField hardcodedFields[] = { PID, USER, PRIORITY, NICE, M_SIZE, M_RESIDENT, M_SHARE, STATE, PERCENT_CPU, PERCENT_MEM, COMM, LAST };

/* private property */
ProcessField initialSelection[LAST] = { PID, USER, PRIORITY, NICE, M_SIZE, M_RESIDENT, M_SHARE, STATE, PERCENT_CPU, PERCENT_MEM, COMM, LAST };

ProcessFilter* ProcessFilter_new() {
   ProcessFilter* this = malloc(sizeof(ProcessFilter));
   ((Object*)this)->class = PROCESSFILTER_CLASS;
   ((Object*)this)->display = ProcessFilter_display;
   this->fields = malloc(sizeof(ProcessField) * LAST);
   // TODO: turn 'fields' into a TypedVector,
   // (and ProcessFields into proper objects).
   for (int i = 0; i < LAST; i++) {
      this->fields[i] = initialSelection[i];
   }
   this->width = 512;
   this->sortKey = PERCENT_CPU;
   this->direction = 1;
   return this;
}

void ProcessFilter_delete(ProcessFilter* this) {
   free(this->fields);
   free(this);
}

void ProcessFilter_invertSortOrder(ProcessFilter* this) {
   if (this->direction == 1)
      this->direction = -1;
   else
      this->direction = 1;
}

void ProcessFilter_sortKey(ProcessFilter* this, int delta) {
   assert(delta == 1 || delta == -1);
   int i = 0;
   while (this->fields[i] != this->sortKey)
      i++;
   i += delta;
   if (i < 0) {
      i = 0;
      while (this->fields[i] != LAST)
         i++;
      i--;
   } else if (this->fields[i] == LAST)
      i = 0;
   this->sortKey = this->fields[i];
   this->direction = 1;
   // Weird code...
}

void ProcessFilter_display(Object* cast, RichString* out) {
   ProcessFilter* this = (ProcessFilter*) cast;
   ProcessField* fields = this->fields;
   RichString_prune(out);
   for (int i = 0; fields[i] != LAST; i++) {
      char* field = ProcessFilter_printHeader(this, fields[i]);
      if (this->sortKey == fields[i])
         RichString_append(out, A_BOLD, field);
      else
         RichString_append(out, A_NORMAL, field);
   }
}

char* ProcessFilter_printHeader(ProcessFilter* this, ProcessField field) {
   switch (field) {
   case PID: return "  PID ";
   case PPID: return " PPID ";
   case PGRP: return " PGRP ";
   case SESSION: return " SESN ";
   case TTY_NR: return "  TTY ";
   case TPGID: return " TGID ";
   case COMM: return "COMM";
   case STATE: return "S ";
   case PRIORITY: return " PR ";
   case NICE: return " NI ";
   case M_SIZE: return " VIRT ";
   case M_RESIDENT: return "  RES ";
   case M_SHARE: return "  SHR ";
   case ST_UID: return " UID ";
   case USER: return "USER     ";
   case UTIME: return "Usr Time ";
   case STIME: return "Sys Time ";
   case PERCENT_CPU: return "CPU% ";
   case PERCENT_MEM: return "MEM% ";
   default: return "- ";
   }
}
