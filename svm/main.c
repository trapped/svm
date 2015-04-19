#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "parser.h"

char* readfull(FILE* input) {
  char* output;
  if(input == stdin) {
    int n = 0;
    int BUFFER_SIZE = 64;
    output = calloc(BUFFER_SIZE, sizeof(char));
    char c;
    while((c = fgetc(input)) != EOF) {
      output[n] = c;
      n++;
      if(n > 0 && !(n % 64)) {
        if(!realloc(output, n + BUFFER_SIZE)) {
          err(1, "error enlargening buffer");
        }
      }
    }
    if(n % 64) {
      if(!realloc(output, n)) {
        err(1, "error fitting buffer");
      }
    }
    realloc(output, n+1);
    output[n] = 0;
    return output;
  } else {
    fseek(input, 0, SEEK_END);
    int size = ftell(input);
    fseek(input, 0, SEEK_SET);
    output = calloc(size + 1, sizeof(char));
    if(!output) {
      err(1, "error allocating buffer");
    }
    fread(output, size, 1, input);
    fclose(input);
    return output;
  }
}

int main(int argc, char** argv) {
  FILE* input;
  if(argc > 1) {
    if(!strcmp(argv[1], "-")) {
      input = stdin;
    } else {
      input = fopen(argv[1], "r");
      if(!input) {
        err(1, "unable to open file for reading");
      }
    }
  } else {
    input = stdin;
  }
  char* code = readfull(input);
  printf("%s\n", code);
  int size = strlen(code);
  printf("size: %d\n", size);
  svm_parser p = { .source_len = size, .source = code, .filename = argv[1], 0 };
  return svm_parse(&p);
}

