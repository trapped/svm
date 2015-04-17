#include <string.h>
#include "dllist.h"

typedef struct svm_parser {
  int       source_len;   /* save source length to save cycles  */
  char*     source;       /* full source text (at pos 0)        */
  int       pos;          /* current position                   */
  char*     filename;
  int       line;
  int       column;
  dl_list*  token_stream; /* double linked list for tokens      */
  int       error;        /* whether an error has occurred      */
} svm_parser;

/* returns the next character without advancing the position */
char svm_parser_seek(svm_parser* p) {
  if(p->pos+1 == p->source_len) {
    return EOF;
  }
  return p->source[p->pos+1];
}

/* returns the next character and advances the position */
char svm_parser_next(svm_parser* p) {
  char c = svm_parser_seek(p);
  if(c == EOF) {
    return EOF;
  }
  p->pos++;
  return c;
}

typedef enum {
  svm_tok_unknown,
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

#define svm_parse_error(p, fmt, ...) do { \
  p->error = 1;                           \
  fprintf(stderr, fmt "\n", __VA_ARGS__); \
} while(0)

/* stateful parser - switch considered harmful! */
int svm_parse(svm_parser* p) {
  if(!p->source_len) {
    p->source_len = strlen(p->source);
  }
  p->token_stream = calloc(1, sizeof(dllist));
  void* next_state = svm_parse_default;
  while(next_state) {
    next_state = next_state(p);
  }
  if(strlen(p->error)) {
    return 0;
  }
  return 1;
}
