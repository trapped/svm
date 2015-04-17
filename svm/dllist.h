typedef struct dl_list {
  void* value;
  struct dl_list* prev;
  struct dl_list* next;
} dl_list;

void dl_push(dl_list** l, void* value) {
  dl_list new = { .value = value, .prev = NULL, .next = *l };
  *l = &new;
}