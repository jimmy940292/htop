/* Do not edit this file. It was automatically genarated. */

#ifndef HEADER_Hashtable
#define HEADER_Hashtable
/*
htop
(C) 2004 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/


#include <stdlib.h>
#include <stdbool.h>

#include "debug.h"

typedef struct Hashtable_ Hashtable;

typedef void(*HashtablePairFunction)(int, void*);
typedef int(*HashtableHashAlgorithm)(Hashtable*, int);

typedef struct HashtableItem {
   int key;
   void* value;
   struct HashtableItem* next;
} HashtableItem;

struct Hashtable_ {
   int size;
   HashtableItem** buckets;
   int items;
   HashtableHashAlgorithm hashAlgorithm;
   bool owner;
};

HashtableItem* HashtableItem_new(int key, void* value);

Hashtable* Hashtable_new(int size, bool owner);

int Hashtable_hashAlgorithm(Hashtable* this, int key);

void Hashtable_delete(Hashtable* this);

inline int Hashtable_size(Hashtable* this);

void Hashtable_put(Hashtable* this, int key, void* value);

void* Hashtable_remove(Hashtable* this, int key);

inline void* Hashtable_get(Hashtable* this, int key);

void Hashtable_foreach(Hashtable* this, HashtablePairFunction f);

#endif