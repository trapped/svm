#ifndef _ERR_H_
#define _ERR_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define warn(...) do {                      \
  int z = fprintf(stderr, ##__VA_ARGS__);     \
  if(z > -1) {                              \
    fprintf(stderr, ": ");                  \
  }                                         \
  fprintf(stderr, "%s\n", strerror(errno)); \
} while(0)

#define err(code, ...) do {                 \
  int z = fprintf(stderr, ##__VA_ARGS__);     \
  if(z > -1) {                              \
    fprintf(stderr, ": ");                  \
  }                                         \
  fprintf(stderr, "%s\n", strerror(errno)); \
  exit(code);                               \
} while(0)

#endif /* !_ERR_H_ */
