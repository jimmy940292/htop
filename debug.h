
#ifdef DEBUG

#include "MemoryMonitor.h"

#define calloc(a, b) MemoryMonitor_calloc(a, b, __FILE__, __LINE__);
#define malloc(x) MemoryMonitor_malloc(x, __FILE__, __LINE__);
#define realloc(x,s) MemoryMonitor_realloc(x, s, __FILE__, __LINE__);
#define strdup(x) MemoryMonitor_strdup(x, __FILE__, __LINE__);
#define free(x) MemoryMonitor_free(x, __FILE__, __LINE__);

#define debug_done() MemoryMonitor_report();

#endif

#ifndef DEBUG

#define NDEBUG
#include <assert.h>
#define debug_done() sleep(0)

#endif
