#include "dl_list.h"

void dl_push(dl_list** l, void* value) {
  dl_list* new = calloc(1, sizeof(dl_list));
  dl_list* first;
  if(!*l) {
    first = new;
  } else {
    first = (*l)->first ? (*l)->first : *l;
  }
  new->value = value;
  new->first = first;
  new->prev  = *l;
  if(*l) {
    (*l)->next = new;
  }
  new->next  = NULL;
  *l = new;
}
