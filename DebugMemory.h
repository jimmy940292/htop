/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_DebugMemory
#define HEADER_DebugMemory

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>


#undef strdup
#undef malloc
#undef realloc
#undef calloc
#undef free

typedef struct DebugMemoryItem_ DebugMemoryItem;

struct DebugMemoryItem_ {
   void* data;
   char* file;
   int line;
   DebugMemoryItem* next;
};

typedef struct DebugMemory_ {
   DebugMemoryItem* first;
   int allocations;
   int deallocations;
   int size;
   FILE* file;
} DebugMemory;


void DebugMemory_new();

void* DebugMemory_malloc(int size, char* file, int line);

void* DebugMemory_calloc(int a, int b, char* file, int line);

void* DebugMemory_realloc(void* ptr, int size, char* file, int line);

void* DebugMemory_strdup(char* str, char* file, int line);

void DebugMemory_free(void* data, char* file, int line);

void DebugMemory_assertSize();

int DebugMemory_getBlockCount();

void DebugMemory_registerAllocation(void* data, char* file, int line);

void DebugMemory_registerDeallocation(void* data, char* file, int line);

void DebugMemory_report();

#endif
