#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"

int readfull(FILE* input, char* output) {
  int n = 0;
  int BUFFER_SIZE = 64;
  output = calloc(BUFFER_SIZE, sizeof(char));
  char c;
  while((c = fgetc(input)) != EOF) {
    output[n] = c;
    n++;
    if(n > 0 && !(n % 64)) {
      if(!realloc(output, n + BUFFER_SIZE)) {
        err(1, NULL);
      }
    }
  }
  if(n % 64) {
    if(!realloc(output, n)) {
      err(1, NULL);
    }
  }
  return n;
}

int main(int argc, char** argv) {
  FILE* input;

  if(argc > 1) {
    if(!strcmp(argv[1], "-")) {
      input = stdin;
    } else {
      input = fopen(argv[1], "rb");
      if(!input) {
        err(1, "unable to open file for reading");
      }
    }
  } else {
    input = stdin;
  }

  char* code = NULL;
  int size = readfull(input, code);
  printf("size: %d\n", size);
  return 0;
}

