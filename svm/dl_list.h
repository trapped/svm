#ifndef DL_LIST_H
#define DL_LIST_H

#include <stdlib.h>

typedef struct dl_list {
  void* value;
  struct dl_list* first;
  struct dl_list* prev;
  struct dl_list* next;
} dl_list;

void dl_push(dl_list**, void*);

#endif
