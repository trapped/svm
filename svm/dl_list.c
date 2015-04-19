#include "dl_list.h"

void dl_push(dl_list** l, void* value) {
  dl_list new = { .value = value, .prev = 0x0, .next = *l };
  *l = &new;
}
