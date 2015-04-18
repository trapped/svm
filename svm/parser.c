#include <stdlib.h>
#include <string.h>
#include "dllist.h"

typedef struct svm_parser {
  int             source_len;   /* save source length to save cycles */
  char*           source;       /* full source text (at pos 0)       */
  int             pos;          /* current position                  */
  char*           filename;
  int             line;
  int             column;
  svm_parser_tok  cur_token;
  dl_list*        token_stream; /* double linked list for tokens     */
  void*           tok_emit_cb;  /* emit() callback                   */
  int             error;        /* whether an error has occurred     */
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

/* moves cur_token to heap and pushes it to token_stream */
void svm_parser_emit(svm_parser* p, svm_parser_tok* tok) {
  svm_parser_tok* nv = calloc(1, sizeof(svm_parser_tok));
  memcpy(nv, tok, sizeof(svm_parser_tok));
  *tok = { 0 };
  dl_push(&p->token_stream, nv);
  char* types[] = { "unknown", "space", "newline", "period", "comma",
    "colon", "percent", "dollar", "hash", "ident", "const" };
  fprintf(stderr, "%s\n", types[nv->type]);
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

/* identifiers or constants */
void* svm_parse_ident_const(svm_parser* p) {
  switch(svm_parser_seek(p)) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':
      if(p->cur_token.type == svm_tok_const) {
        /* a constant outside quotes is a number, which can't contain
           letters */
        p->cur_token.type = svm_tok_ident;
      } else if(p->cur_token.type == svm_tok_unknown) {
        /* if it starts with a letter it's for sure an ident */
        p->cur_token.type = svm_tok_ident;
      }
      /* fallthrough */
    case '0' ... '9':
      if(p->cur_token.type == svm_tok_unknown) {
        /* holds as long as there are no letters */
        p->cur_token.type = svm_tok_const;
      }
      /* fallthrough */
      svm_parser_next(p);
      p->cur_token.end_pos++;
      return svm_parse_ident_const;
    case EOF:
      svm_parse_error("unexpected EOF");
      return NULL;
    default:
    svm_parser_emit(p, &p->cur_token);
      return svm_parse_default;
  }
}

/* stateful parser - switch considered harmful! */
int svm_parse(svm_parser* p) {
  if(!p->source_len) {
    p->source_len = strlen(p->source);
  }
  p->token_stream = calloc(1, sizeof(dllist));
  p->cur_token = { 0 };
  void* next_state = svm_parse_default;
  while(next_state) {
    next_state = next_state(p);
  }
  if(strlen(p->error)) {
    return 0;
  }
  return 1;
}
