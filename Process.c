/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "Process.h"
#include "ProcessFilter.h"
#include "Object.h"
#include "CRT.h"
#include "String.h"

#include "debug.h"

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <pwd.h>

// TODO: wtf!?
int kill(pid_t pid, int signal);

// This works only with glibc 2.1+. On earlier versions
// the behavior is similar to have a hardcoded page size.
#define PAGE_SIZE ( sysconf(_SC_PAGESIZE) / 1024 )

#define PROCESS_COMM_LEN 512
#define PROCESS_USER_LEN 10

/*{
typedef struct Process_ {
   Object super;

   ProcessFilter* filter;
   bool updated;

   int pid;
   char comm[PROCESS_COMM_LEN + 2];
   char state;
   bool tag;
   int ppid;
   int pgrp;
   int session;
   int tty_nr;
   int tpgid;
   unsigned long int flags;
   unsigned long int minflt;
   unsigned long int cminflt;
   unsigned long int majflt;
   unsigned long int cmajflt;
   unsigned long int utime;
   unsigned long int stime;
   long int cutime;
   long int cstime;
   long int priority;
   long int nice;
   long int itrealvalue;
   unsigned long int starttime;
   unsigned long int vsize;
   long int rss;
   unsigned long int rlim;
   unsigned long int startcode;
   unsigned long int endcode;
   unsigned long int startstack;
   unsigned long int kstkesp;
   unsigned long int kstkeip;
   unsigned long int signal;
   unsigned long int blocked;
   unsigned long int sigignore;
   unsigned long int sigcatch;
   unsigned long int wchan;
   unsigned long int nswap;
   unsigned long int cnswap;
   int exit_signal;
   int processor;
   int m_size;
   int m_resident;
   int m_share;
   int m_trs;
   int m_drs;
   int m_lrs;
   int m_dt;
   uid_t st_uid;
   float percent_cpu;
   float percent_mem;
   char user[PROCESS_USER_LEN + 1];
} Process;

extern char* PROCESS_CLASS;
}*/

/* private property */
char* PROCESS_CLASS = "Process";

Process* Process_new(ProcessFilter* filter) {
   Process* this = malloc(sizeof(Process));
   ((Object*)this)->class = PROCESS_CLASS;
   ((Object*)this)->display = Process_display;
   ((Object*)this)->equals = Process_equals;
   ((Object*)this)->delete = Process_delete;
   this->filter = filter;
   this->tag = false;
   this->updated = false;
   this->utime = 0;
   this->stime = 0;
   return this;
}

Process* Process_clone(Process* this) {
   Process* clone = malloc(sizeof(Process));
   memcpy(clone, this, sizeof(Process));
   return clone;
}

void Process_delete(Object* cast) {
   Process* this = (Process*) cast;
   assert (this != NULL);
   free(this);
}

void Process_display(Object* cast, RichString* out) {
   Process* this = (Process*) cast;
   ProcessField* fields = this->filter->fields;
   char buffer[PROCESS_COMM_LEN];
   RichString_prune(out);
   for (int i = 0; fields[i] != LAST; i++) {
      int attr = Process_writeField(this, buffer, fields[i]);
      if (this->tag == true)
         attr = A_BOLD | COLOR_PAIR(BROWN_PAIR);
      RichString_append(out, attr, buffer);
   }
   assert(out->len > 0);
}

void Process_toggleTag(Process* this) {
   this->tag = this->tag == true ? false : true;
}

bool Process_equals(const Object* o1, const Object* o2) {
   Process* p1 = (Process*) o1;
   Process* p2 = (Process*) o2;
   if (p1 == NULL || p2 == NULL)
      return false;
   return (p1->pid == p2->pid);
}

void Process_setPriority(Process* this, int priority) {
   int err = setpriority(PRIO_PROCESS, this->pid, priority);
   if (err == 0) {
      this->nice = priority;
   }
}

void Process_sendSignal(Process* this, int signal) {
   kill(this->pid, signal);
}

#define ONE_K 1024
#define ONE_M (ONE_K * ONE_K)
#define ONE_G (ONE_M * ONE_K)

/* private */
void Process_printLargeNumber(char *out, int len, unsigned int number) {
   if(number > (1000 * ONE_M))
      snprintf(out, len, "%4.2fG ", (float)number / ONE_M);
   else if(number > (99 * ONE_K))
      snprintf(out, len, "%4dM ", number / ONE_K);
   else
      snprintf(out, len, "%5d ", number);
}

/* private property */
double jiffy = 0.0;

/* private */
void Process_printTime(char *out, int len, unsigned long t) {
   if(jiffy == 0.0) jiffy = sysconf(_SC_CLK_TCK);
   double jiffytime = 1.0 / jiffy;

   double realTime = t * jiffytime;
   int iRealTime = (int) realTime;

   snprintf(out, len, "%02d:%02d:%02d ", iRealTime / 3600, 
   	(iRealTime / 60) % 60, iRealTime % 60);
}

int Process_writeField(Process* this, char* out, ProcessField field) {
   int n = PROCESS_COMM_LEN;
   switch (field) {
   case PID: snprintf(out, n, "%5d ", this->pid); return A_NORMAL;
   case PPID: snprintf(out, n, "%5d ", this->ppid); return A_NORMAL;
   case PGRP: snprintf(out, n, "%5d ", this->pgrp); return A_NORMAL;
   case SESSION: snprintf(out, n, "%5d ", this->session); return A_NORMAL;
   case TTY_NR: snprintf(out, n, "%5d ", this->tty_nr); return A_NORMAL;
   case TPGID: snprintf(out, n, "%5d ", this->tpgid); return A_NORMAL;
   case COMM: snprintf(out, n, "%s ", this->comm); return A_NORMAL;
   case STATE: snprintf(out, n, "%c ", this->state); return (this->state == 'R' ? COLOR_PAIR(GREEN_PAIR) : A_NORMAL);
   case PRIORITY:
      if(this->priority == -100)
         snprintf(out, n, " RT ");
      else
         snprintf(out, n, "%3ld ", this->priority);
      return A_NORMAL;
   case NICE:
      snprintf(out, n, "%3ld ", this->nice);
      if (this->nice == 0)
         return A_NORMAL;
      if (this->nice < 0)
         return COLOR_PAIR(RED_PAIR);
      else
         return COLOR_PAIR(CYAN_PAIR);
   case M_SIZE: Process_printLargeNumber(out, n, this->m_size * PAGE_SIZE); return A_NORMAL;
   case M_RESIDENT: Process_printLargeNumber(out, n, this->m_resident * PAGE_SIZE); return A_NORMAL;
   case M_SHARE: Process_printLargeNumber(out, n, this->m_share * PAGE_SIZE); return A_NORMAL;
   case ST_UID: snprintf(out, n, "%4d ", this->st_uid); return A_NORMAL;
   case USER: {
      snprintf(out, n, "%-8s ", this->user);
      if (out[8] != '\0') {
         out[8] = ' ';
         out[9] = '\0';
      }
      if (this->st_uid == getuid())
         return A_NORMAL;
      else
         return COLOR_PAIR(BLACK_PAIR) | A_BOLD;
   }
   case UTIME: Process_printTime(out, n, this->utime); return A_NORMAL;
   case STIME: Process_printTime(out, n, this->stime); return A_NORMAL;
   case PERCENT_CPU: {
      if (this->percent_cpu > 99.9) {
         snprintf(out, n, "100. "); 
      } else {
         snprintf(out, n, "%4.1f ", this->percent_cpu);
      }
      return A_NORMAL;
   }
   case PERCENT_MEM: {
      if (this->percent_mem > 99.9) {
         snprintf(out, n, "100. "); 
      } else {
         snprintf(out, n, "%4.1f ", this->percent_mem);
      }
      return A_NORMAL;
   }
   default:
      snprintf(out, n, "- "); return A_NORMAL;
   }
   return A_NORMAL;
}

int Process_compare(const Object* v1, const Object* v2) {
   Process* p1 = (Process*)v1;
   Process* p2 = (Process*)v2;
   int direction = p1->filter->direction;
   switch (p1->filter->sortKey) {
   case PID:
      return (p2->pid - p1->pid) * direction;
   case PPID:
      return (p2->ppid - p1->ppid) * direction;
   case USER:
      return strcmp(p2->user, p1->user) * direction;
   case PRIORITY:
      return (p2->priority - p1->priority) * direction;
   case STATE:
      return (p2->state - p1->state) * direction;
   case NICE:
      return (p2->nice - p1->nice) * direction;
   case M_SIZE:
      return (p1->m_size - p2->m_size) * direction;
   case M_RESIDENT:
      return (p1->m_resident - p2->m_resident) * direction;
   case M_SHARE:
      return (p1->m_share - p2->m_share) * direction;
   case PERCENT_CPU:
      return (p1->percent_cpu < p2->percent_cpu ? -1 : 1) * direction;
   case PERCENT_MEM:
      return (p1->percent_mem < p2->percent_mem ? -1 : 1) * direction;
   case COMM:
      return strcmp(p2->comm, p1->comm) * direction;
   default:
      return (p2->pid - p1->pid) * direction;
   }
}
