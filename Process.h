/* Do not edit this file. It was automatically generated. */

#ifndef HEADER_Process
#define HEADER_Process
/*
htop - Process.h
(C) 2004-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#define _GNU_SOURCE
#include "ProcessList.h"
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
#include <sched.h>

// This works only with glibc 2.1+. On earlier versions
// the behavior is similar to have a hardcoded page size.
#ifndef PAGE_SIZE
#define PAGE_SIZE ( sysconf(_SC_PAGESIZE) / 1024 )
#endif

#define PROCESS_COMM_LEN 300


typedef enum ProcessField_ {
   PID = 1, COMM, STATE, PPID, PGRP, SESSION, TTY_NR, TPGID, FLAGS, MINFLT, CMINFLT, MAJFLT, CMAJFLT, UTIME,
   STIME, CUTIME, CSTIME, PRIORITY, NICE, ITREALVALUE, STARTTIME, VSIZE, RSS, RLIM, STARTCODE, ENDCODE,
   STARTSTACK, KSTKESP, KSTKEIP, SIGNAL, BLOCKED, SSIGIGNORE, SIGCATCH, WCHAN, NSWAP, CNSWAP, EXIT_SIGNAL,
   PROCESSOR, M_SIZE, M_RESIDENT, M_SHARE, M_TRS, M_DRS, M_LRS, M_DT, ST_UID, PERCENT_CPU, PERCENT_MEM,
   USER, TIME, NLWP, TGID,
   #ifdef HAVE_OPENVZ
   VEID, VPID,
   #endif
   LAST_PROCESSFIELD
} ProcessField;

struct ProcessList_;

typedef struct Process_ {
   Object super;

   struct ProcessList_ *pl;
   bool updated;

   unsigned int pid;
   char* comm;
   int indent;
   char state;
   bool tag;
   unsigned int ppid;
   unsigned int pgrp;
   unsigned int session;
   unsigned int tty_nr;
   unsigned int tgid;
   int tpgid;
   unsigned long int flags;
   #ifdef DEBUG
   unsigned long int minflt;
   unsigned long int cminflt;
   unsigned long int majflt;
   unsigned long int cmajflt;
   #endif
   unsigned long int utime;
   unsigned long int stime;
   long int cutime;
   long int cstime;
   long int priority;
   long int nice;
   long int nlwp;
   #ifdef DEBUG
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
   #endif
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
   char* user;
   #ifdef HAVE_OPENVZ
   unsigned int veid;
   unsigned int vpid;
   #endif
} Process;


#ifdef DEBUG
extern char* PROCESS_CLASS;
#else
#define PROCESS_CLASS NULL
#endif

extern char *Process_fieldNames[];

Process* Process_new(struct ProcessList_ *pl);

Process* Process_clone(Process* this);

void Process_delete(Object* cast);

void Process_display(Object* cast, RichString* out);

void Process_toggleTag(Process* this);

void Process_setPriority(Process* this, int priority);

unsigned long Process_getAffinity(Process* this);

void Process_setAffinity(Process* this, unsigned long mask);

void Process_sendSignal(Process* this, int signal);

#define ONE_K 1024
#define ONE_M (ONE_K * ONE_K)
#define ONE_G (ONE_M * ONE_K)

void Process_writeField(Process* this, RichString* str, ProcessField field);

int Process_pidCompare(const void* v1, const void* v2);

int Process_compare(const void* v1, const void* v2);

char* Process_printField(ProcessField field);

#endif
