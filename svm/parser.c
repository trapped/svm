#include <string.h>
#include "dllist.h"

typedef struct svm_parser {
  char*     source;       /* full source text (at pos 0)   */
  int       pos;          /* current position              */
  int       line;
  int       column;
  dl_list*  token_stream; /* double linked list for tokens */
  char*     error;        /* contains the error string     */
} svm_parser;

typedef enum {
  svm_tok_space,    /*      */
  svm_tok_newline,  /* \r\n */
  svm_tok_period,   /* .    */
  svm_tok_comma,    /* ,    */
  svm_tok_colon,    /* :    */
  svm_tok_percent,  /* %    */
  svm_tok_dollar,   /* $    */
  svm_tok_hash,     /* #    */
  svm_tok_ident,    /* func */
  svm_tok_const     /* 1 "" */
} svm_tok_type;

typedef struct {
  svm_tok_type  type;
  int           start_pos;
  int           end_pos;
  /* value is sliced from source text */
} svm_parser_tok;

/* stateful parser - switch considered harmful! */
int svm_parse(svm_parser* p) {
  void* next_state = a;
  while(next_state) {
    next_state = next_state(p);
  }
  if(strlen(p->error)) {
    return 0;
  }
  return 1;
}
