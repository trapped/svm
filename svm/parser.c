#include "parser.h"

/* returns the next character without advancing the position */
char svm_parser_seek(svm_parser* p) {
  if(p->pos+1 >= p->source_len) {
    return EOF;
  }
  return p->source[p->pos+1];
}

/* returns the next character and advances the position */
char svm_parser_next(svm_parser* p) {
  char c = svm_parser_seek(p);
  if(c == EOF) {
    p->pos++;
    return EOF;
  }
  p->column++;
  p->pos++;
  return c;
}

/* goes back a character */
char svm_parser_prev(svm_parser* p) {
  if(p->pos-1 < 0) {
    return EOF;
  }
  p->column--;
  p->pos--;
  return p->source[p->pos];
}

void svm_tok_print(svm_parser* p, svm_parser_tok* t) {
  char* types[] = { "unknown", "newline", "period", "comma",
    "colon", "equal", "percent", "dollar", "ident", "const", "comment" };
  fprintf(stderr, "%s:%d-%d(%d): '%.*s'\n", types[t->type], t->start_pos, t->end_pos,
    t->end_pos - t->start_pos, t->end_pos - t->start_pos, &p->source[t->start_pos]);
}

/* moves cur_token to heap and pushes it to token_stream */
void svm_parser_emit(svm_parser* p, svm_parser_tok* tok) {
  svm_parser_tok* nv = calloc(1, sizeof(svm_parser_tok));
  memcpy(nv, tok, sizeof(svm_parser_tok));
  svm_parser_tok tmp = { 0 };
  memcpy(tok, &tmp, sizeof(svm_parser_tok)); /* it appears that you can't reset using a compound literal */
  dl_push(&p->token_stream, nv);
  char* types[] = { "unknown", "space", "newline", "period", "comma",
    "colon", "percent", "dollar", "hash", "ident", "const" };
  fprintf(stderr, "%s\n", types[nv->type]);
}

#define svm_parse_error(p, fmt, ...) do { \
  p->error = 1;                           \
  fprintf(stderr, "%s %d:%d: " fmt "\n",  \
    p->filename, p->line, p->column,      \
    ##__VA_ARGS__);                       \
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
      svm_parse_error(p, "unexpected EOF");
      return NULL;
    default:
      svm_parser_emit(p, &p->cur_token);
      return svm_parse_default;
  }
}

/* comments */
void* svm_parse_comment(svm_parser* p) {
  switch(svm_parser_seek(p)) {
    case '\n':
      return svm_parse_default;
    default:
      svm_parser_next(p);
      return svm_parse_comment;
  }
}

/* stateful parser - switch considered harmful! */
int svm_parse(svm_parser* p) {
  if(!p->source) {
    err(1, "source is null");
  }
  if(!p->source_len) {
    p->source_len = strlen(p->source);
  }
  p->token_stream = calloc(1, sizeof(dl_list));
  void* next_state = svm_parse_default;
  while(next_state) {
    next_state = (*(void*(*)())next_state)(p);
  }
  if(p->error) {
    return 0;
  }
  return 1;
}
