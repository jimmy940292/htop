/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ProcessList.h"
#include "Process.h"
#include "ProcessFilter.h"
#include "TypedVector.h"
#include "UsersTable.h"
#include "Hashtable.h"

#include "debug.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>

/*{
#ifndef PROCDIR
#define PROCDIR "/proc"
#endif

#ifndef PROCSTATFILE
#define PROCSTATFILE "/proc/stat"
#endif

#ifndef PROCMEMINFOFILE
#define PROCMEMINFOFILE "/proc/meminfo"
#endif
}*/

/*{
typedef struct ProcessList_ {
   TypedVector* processes;
   Hashtable* processTable;
   ProcessFilter* filter;
   Process* prototype;
   UsersTable* usersTable;
   long int totalTime;
   long int userTime;
   long int systemTime;
   long int idleTime;
   long int niceTime;
   long int totalPeriod;
   long int userPeriod;
   long int systemPeriod;
   long int idlePeriod;
   long int nicePeriod;
   long int totalMem;
   long int usedMem;
   long int freeMem;
   long int sharedMem;
   long int buffersMem;
   long int cachedMem;
   long int totalSwap;
   long int usedSwap;
   long int freeSwap;
} ProcessList;
}*/

ProcessList* ProcessList_new(ProcessFilter* filter, UsersTable* usersTable) {
   ProcessList* this;
   this = malloc(sizeof(ProcessList));
   this->processes = TypedVector_new(PROCESS_CLASS, true);
   this->processTable = Hashtable_new(20, false);
   TypedVector_setCompareFunction(this->processes, Process_compare);
   this->filter = filter;
   this->prototype = Process_new(this->filter);
   this->totalTime = 0;
   this->usersTable = usersTable;
   return this;
}

void ProcessList_delete(ProcessList* this) {
   Hashtable_delete(this->processTable);
   TypedVector_delete(this->processes);
   Process_delete((Object*)this->prototype);
   free(this);
}

void ProcessList_prune(ProcessList* this) {
   TypedVector_prune(this->processes);
}

void ProcessList_add(ProcessList* this, Process* p) {
   TypedVector_add(this->processes, p);
   Hashtable_put(this->processTable, p->pid, p);
}

void ProcessList_remove(ProcessList* this, Process* p) {
   Hashtable_remove(this->processTable, p->pid);
   int index = TypedVector_indexOf(this->processes, p);
   TypedVector_remove(this->processes, index);
}

Process* ProcessList_get(ProcessList* this, int index) {
   return (Process*) (TypedVector_get(this->processes, index));
}

int ProcessList_size(ProcessList* this) {
   return (TypedVector_size(this->processes));
}

void ProcessList_sort(ProcessList* this) {
   TypedVector_sort(this->processes);
}

/* private */
int ProcessList_readStatFile(Process *proc, FILE *f, char *command) {
   #define MAX_READ 8192
   static char buf[MAX_READ];
   long int zero;

   int size = fread(buf, 1, MAX_READ, f);
   if(!size) return 0;

   proc->pid = atoi(buf);
   char *location = strchr(buf, ' ');
   if(!location) return 0;

   location += 2;
   char *end = strrchr(location, ')');
   if(!end) return 0;
   
   int commsize = end - location;
   memcpy(command, location, commsize);
   command[commsize] = '\0';
   location = end + 2;
   
   int num = sscanf(location, 
      "%c %d %d %d %d %d %lu %lu %lu %lu "
      "%lu %lu %lu %ld %ld %ld %ld %ld %ld "
      "%lu %lu %ld %lu %lu %lu %lu %lu "
      "%lu %lu %lu %lu %lu %lu %lu %lu "
      "%d %d",
      &proc->state, &proc->ppid, &proc->pgrp, &proc->session, &proc->tty_nr, 
      &proc->tpgid, &proc->flags, &proc->minflt, &proc->cminflt, &proc->majflt, 
      &proc->cmajflt, &proc->utime, &proc->stime, &proc->cutime, &proc->cstime, 
      &proc->priority, &proc->nice, &zero, &proc->itrealvalue, 
      &proc->starttime, &proc->vsize, &proc->rss, &proc->rlim, 
      &proc->startcode, &proc->endcode, &proc->startstack, &proc->kstkesp, 
      &proc->kstkeip, &proc->signal, &proc->blocked, &proc->sigignore, 
      &proc->sigcatch, &proc->wchan, &proc->nswap, &proc->cnswap, 
      &proc->exit_signal, &proc->processor);
   
   // This assert is always valid on 2.4, but _not always valid_ on 2.6.
   // TODO: Check if the semantics of this field has changed.
   // assert(zero == 0);
   
   if(num != 37) return 0;
   return 1;
}

void ProcessList_scan(ProcessList* this) {
   DIR* proc;
   struct dirent* entry;
   Process* prototype = this->prototype;
   long int usertime, nicetime, systemtime, idletime, totaltime;

   FILE* status;
   char buffer[128];
   status = fopen(PROCMEMINFOFILE, "r");
   assert(status != NULL);
   while (!feof(status)) {
      fgets(buffer, 128, status);
      if (String_startsWith(buffer, "MemTotal:")) {
         sscanf(buffer, "MemTotal: %ld kB", &this->totalMem);
      } else if (String_startsWith(buffer, "MemFree:")) {
         sscanf(buffer, "MemFree: %ld kB", &this->freeMem);
         this->usedMem = this->totalMem - this->freeMem;
      } else if (String_startsWith(buffer, "MemShared:")) {
         sscanf(buffer, "MemShared: %ld kB", &this->sharedMem);
      } else if (String_startsWith(buffer, "Buffers:")) {
         sscanf(buffer, "Buffers: %ld kB", &this->buffersMem);
      } else if (String_startsWith(buffer, "Cached:")) {
         sscanf(buffer, "Cached: %ld kB", &this->cachedMem);
      } else if (String_startsWith(buffer, "SwapTotal:")) {
         sscanf(buffer, "SwapTotal: %ld kB", &this->totalSwap);
      } else if (String_startsWith(buffer, "SwapFree:")) {
         long int swapFree;
         sscanf(buffer, "SwapFree: %ld kB", &swapFree);
         this->usedSwap = this->totalSwap - swapFree;
      }
   }
   fclose(status);

   status = fopen(PROCSTATFILE, "r");
   assert(status != NULL);
   fscanf(status, "cpu  %ld %ld %ld %ld", &usertime, &nicetime, &systemtime, &idletime);
   totaltime = usertime + nicetime + systemtime + idletime;
   long int totalperiod = totaltime - this->totalTime;
   fclose(status);

   // mark all process as "dirty"
   for (int i = 0; i < TypedVector_size(this->processes); i++) {
      Process* p = (Process*) TypedVector_get(this->processes, i);
      p->updated = false;
   }

   proc = opendir(PROCDIR);
   assert(proc != NULL);
   signal(11, ProcessList_dontCrash);
   while ((entry = readdir(proc)) != NULL) {
      char* name = entry->d_name;
      int pid;
      // filename is a number: process directory
      pid = atoi(name);

      // The RedHat kernel hides threads with a dot.
      // I believe this is non-standard.
      bool isThread = false;
      if (pid == 0 && name[0] == '.') {
         char* tname = name + 1;
         pid = atoi(tname);
         if (pid > 0)
            isThread = true;
      }

      if (pid > 0) {
         FILE* status;
         const int MAX_NAME = 128;
         char statusfilename[MAX_NAME+1];
         char command[PROCESS_COMM_LEN + 1];

         Process* process;
         Process* existingProcess = (Process*) Hashtable_get(this->processTable, pid);
         if (!existingProcess) {
            process = Process_clone(prototype);
            process->pid = pid;
            ProcessList_add(this, process);
         } else {
            process = existingProcess;
         }
         process->updated = true;

         struct stat sstat;
         snprintf(statusfilename, MAX_NAME, "%s/%s/stat", PROCDIR, name);
         stat(statusfilename, &sstat);
         char* username = UsersTable_getRef(this->usersTable, sstat.st_uid);
         if (username) {
            strncpy(process->user, username, PROCESS_USER_LEN);
         } else {
            snprintf(process->user, PROCESS_USER_LEN, "%d", sstat.st_uid);
         }
         process->st_uid = sstat.st_uid;

         int lasttimes = (process->utime + process->stime);

         status = fopen(statusfilename, "r");
         if (status == NULL) 
            goto errorReadingProcess;
         
         int success = ProcessList_readStatFile(process, status, command);
         fclose(status);
         if(!success) {
            goto errorReadingProcess;
         }
         
         process->percent_cpu = (process->utime + process->stime - lasttimes) / 
            (float)totalperiod * 100.0;

         if(!existingProcess) {
            snprintf(statusfilename, MAX_NAME, "%s/%s/cmdline", PROCDIR, name);
            status = fopen(statusfilename, "r");
            if (!status) {
               goto errorReadingProcess;
            }

            char* cmdline = process->comm;
            int amtRead = fread(cmdline, 1, PROCESS_COMM_LEN - 1, status);
            if (amtRead > 0) {
               for (int i = 0; i < amtRead; i++)
                  if (cmdline[i] == '\0' || cmdline[i] == '\n')
                     cmdline[i] = ' ';
               cmdline[amtRead] = '\0';
            }
            else {
               strncpy(process->comm, command, PROCESS_COMM_LEN);
               cmdline[PROCESS_COMM_LEN] = '\0';
            }
            fclose(status);
         }

         snprintf(statusfilename, MAX_NAME, "%s/%s/statm", PROCDIR, name);
         status = fopen(statusfilename, "r");
         if(!status) {
            goto errorReadingProcess;
         }
         int num = fscanf(status, "%d %d %d %d %d %d %d",
             &process->m_size, &process->m_resident, &process->m_share, 
             &process->m_trs, &process->m_drs, &process->m_lrs, 
             &process->m_dt);
         fclose(status);
         if(num != 7)
            goto errorReadingProcess;

         process->percent_mem = process->m_resident / 
            (float)(this->usedMem - this->cachedMem - this->buffersMem) * 
            100.0;
         continue;

         // Exception handler.
         errorReadingProcess: {
            ProcessList_remove(this, process);
         }
      }
   }
   signal(11, SIG_DFL);
   closedir(proc);
   
   for (int i = TypedVector_size(this->processes) - 1; i >= 0; i--) {
      Process* p = (Process*) TypedVector_get(this->processes, i);
      if (p->updated == false)
         ProcessList_remove(this, p);
      else
         p->updated = false;
   }

   totalperiod = totaltime - this->totalTime;
   long int userperiod = usertime - this->userTime;
   long int niceperiod = nicetime - this->niceTime;
   long int systemperiod = systemtime - this->systemTime;
   long int idleperiod = idletime - this->idleTime;

   this->totalTime = totaltime;
   this->userTime = usertime;
   this->niceTime = nicetime;
   this->systemTime = systemtime;
   this->idleTime = idletime;

   this->totalPeriod = totalperiod;
   this->userPeriod = userperiod;
   this->nicePeriod = niceperiod;
   this->systemPeriod = systemperiod;
   this->idlePeriod = idleperiod;
}

void ProcessList_dontCrash(int signal) {
   // This ugly hack was added because I suspect some
   // crashes were caused by contents of /proc vanishing
   // away while we read them.
}
