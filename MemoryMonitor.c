
#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "MemoryMonitor.h"

#undef strdup
#undef malloc
#undef realloc
#undef calloc
#undef free

/*{
typedef struct MemoryMonitorItem_ MemoryMonitorItem;

struct MemoryMonitorItem_ {
   void* data;
   char* file;
   int line;
   MemoryMonitorItem* next;
};

typedef struct MemoryMonitor_ {
   MemoryMonitorItem* first;
   int allocations;
   int deallocations;
   int size;
} MemoryMonitor;
}*/

/* private property */
MemoryMonitor* singleton = NULL;

void MemoryMonitor_new() {
   if (singleton)
      return;
   singleton = malloc(sizeof(MemoryMonitor));
   singleton->first = NULL;
   singleton->allocations = 0;
   singleton->deallocations = 0;
   singleton->size = 0;
}

void* MemoryMonitor_malloc(int size, char* file, int line) {
   void* data = malloc(size);
   MemoryMonitor_registerAllocation(data, file, line);
   return data;
}

void* MemoryMonitor_calloc(int a, int b, char* file, int line) {
   void* data = calloc(a, b);
   MemoryMonitor_registerAllocation(data, file, line);
   return data;
}

void* MemoryMonitor_realloc(void* ptr, int size, char* file, int line) {
   if (ptr != NULL)
      MemoryMonitor_registerDeallocation(ptr, file, line);
   void* data = realloc(ptr, size);
   MemoryMonitor_registerAllocation(data, file, line);
   return data;
}

void* MemoryMonitor_strdup(char* str, char* file, int line) {
   char* data = strdup(str);
   MemoryMonitor_registerAllocation(data, file, line);
   return data;
}

void MemoryMonitor_free(void* data, char* file, int line) {
   MemoryMonitor_registerDeallocation(data, file, line);
   free(data);
}

void MemoryMonitor_assertSize() {
   if (!singleton->first) {
      assert (singleton->size == 0);
   }
   MemoryMonitorItem* walk = singleton->first;
   int i = 0;
   while (walk != NULL) {
      i++;
      walk = walk->next;
   }
   assert (i == singleton->size);
}

int MemoryMonitor_getBlockCount() {
   if (!singleton->first) {
      return 0;
   }
   MemoryMonitorItem* walk = singleton->first;
   int i = 0;
   while (walk != NULL) {
      i++;
      walk = walk->next;
   }
   return i;
}

void MemoryMonitor_registerAllocation(void* data, char* file, int line) {
   if (!singleton)
      MemoryMonitor_new();
   MemoryMonitor_assertSize();
   MemoryMonitorItem* item = (MemoryMonitorItem*) malloc(sizeof(MemoryMonitorItem));
   item->data = data;
   item->file = file;
   item->line = line;
   item->next = NULL;
   int val = MemoryMonitor_getBlockCount();
   if (singleton->first == NULL) {
      assert (val == 0);
      singleton->first = item;
   } else {
      MemoryMonitorItem* walk = singleton->first;
      while (true) {
         if (walk->next == NULL) {
            walk->next = item;
            break;
         }
         walk = walk->next;
      }
   }
   int nval = MemoryMonitor_getBlockCount();
   assert(nval == val + 1);
   singleton->allocations++;
   singleton->size++;
   MemoryMonitor_assertSize();
}

void MemoryMonitor_registerDeallocation(void* data, char* file, int line) {
   assert(singleton);
   assert(singleton->first);
   MemoryMonitorItem* walk = singleton->first;
   MemoryMonitorItem* prev = NULL;
   int val = MemoryMonitor_getBlockCount();
   while (walk != NULL) {
      if (walk->data == data) {
         if (prev == NULL) {
            singleton->first = walk->next;
         } else {
            prev->next = walk->next;
         }
         free(walk);
         assert(MemoryMonitor_getBlockCount() == val - 1);
         singleton->deallocations++;
         singleton->size--;
         MemoryMonitor_assertSize();
         return;
      }
      MemoryMonitorItem* tmp = walk;
      walk = walk->next;
      prev = tmp;
   }
   MemoryMonitor_report();
   fprintf(stderr, "Couldn't find allocation for memory freed at %s:%d\n", file, line);
   assert(false);
}

void MemoryMonitor_report() {
   assert(singleton);
   MemoryMonitorItem* walk = singleton->first;
   int i = 0;
   while (walk != NULL) {
      i++;
      fprintf(stderr, "%p %s:%d\n", walk->data, walk->file, walk->line);
      walk = walk->next;
   }
   fprintf(stderr, "Total:\n");
   fprintf(stderr, "%d allocations\n", singleton->allocations);
   fprintf(stderr, "%d deallocations\n", singleton->deallocations);
   fprintf(stderr, "%d size\n", singleton->size);
   fprintf(stderr, "%d non-freed blocks\n", i);
}
