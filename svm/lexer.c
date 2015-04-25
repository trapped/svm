#include "lexer.h"

void* lex_ident_const(svm_lexer*);
void* lex_comment(svm_lexer*);
void* lex_default(svm_lexer*);

/* returns the next character without advancing the position */
char seek(svm_lexer* p) {
  if(p->pos+1 >= p->source_len) {
    return EOF;
  }
  return p->source[p->pos+1];
}

/* returns the next character and advances the position */
char next(svm_lexer* p) {
  char c = seek(p);
  if(c == EOF) {
    p->pos++;
    return EOF;
  }
  p->column++;
  p->pos++;
  return c;
}

/* goes back a character */
char previous(svm_lexer* p) {
  if(p->pos-1 < 0) {
    return EOF;
  }
  p->column--;
  p->pos--;
  return p->source[p->pos];
}

/* ignores the next character */
void ignore(svm_lexer* p) {
  p->cur_token.start_pos++;
}

/* prints a token to stderr */
void svm_tok_print(svm_lexer* p, svm_lexer_tok* t) {
  char* types[] = { "unknown", "newline", "period", "comma", "colon", "equal",
  "percent", "dollar", "opbrak", "clbrak", "ident", "const", "comment" };
  fprintf(stderr, "%s:%d-%d(%d): '%.*s'\n", types[t->type], t->start_pos, t->end_pos,
    t->end_pos - t->start_pos, t->type == svm_tok_newline ? 2 : t->end_pos - t->start_pos,
    t->type == svm_tok_newline ? "\\n" : &p->source[t->start_pos]);
}

/* moves cur_token to heap and pushes it to token_stream */
void emit(svm_lexer* p) {
  p->cur_token.end_pos = p->pos;
  svm_lexer_tok* nv = calloc(1, sizeof(svm_lexer_tok));
  *nv = p->cur_token;
  /* it appears that you can't reset using a compound literal directly */
  svm_lexer_tok tmp = { 0, .start_pos = p->pos, .end_pos = p->pos };
  p->cur_token = tmp;
  dl_push(&p->token_stream, nv);
  //svm_tok_print(p, nv);
}

/* emits and advances position */
void emit_advance(svm_lexer* p) {
  next(p);
  emit(p);
  previous(p);
}

#define lex_error(p, fmt, ...) do { \
  p->error = 1;                           \
  fprintf(stderr, "%s:%d:%d: " fmt "\n",  \
    p->filename, p->line, p->column,      \
    ##__VA_ARGS__);                       \
} while(0)
//backtrace_symbols_fd(1, 1, stdout); only on GNU/Linux?

/* identifiers or constants */
void* lex_ident_const(svm_lexer* p) {
  char c;
  switch(c = next(p)) {
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
      return lex_ident_const;
    case EOF:
      lex_error(p, "unexpected EOF");
      return NULL;
    default:
      emit(p);
      previous(p); /* we haven't processed it after all */
      return lex_default;
  }
}

/* comments */
void* lex_comment(svm_lexer* p) {
  switch(next(p)) {
    case '\n':
      emit(p);
      previous(p); /* let the \n be lexd */
      return lex_default;
    default:
      return lex_comment;
  }
}

/* anything (mostly dispatches) */
void* lex_default(svm_lexer* p) {
  char c;
  switch(c = next(p)) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '0' ... '9':
    case '_':
      previous(p);
      return lex_ident_const;
    case '#':
      p->cur_token.type = svm_tok_comment;
      /* ignore hash character */
      ignore(p);
      return lex_comment;
    case '\n':
      p->line++;
      p->column = 0;
      p->cur_token.type = svm_tok_newline;
      emit_advance(p);
      return lex_default;
    case '\r':
      ignore(p);
      return lex_default;
    case '.':
      p->cur_token.type = svm_tok_period;
      emit_advance(p);
      return lex_default;
    case ':':
      p->cur_token.type = svm_tok_colon;
      emit_advance(p);
      return lex_default;
    case '=':
      p->cur_token.type = svm_tok_equal;
      emit_advance(p);
      return lex_default;
    case ',':
      p->cur_token.type = svm_tok_comma;
      emit_advance(p);
      return lex_default;
    case '%':
      p->cur_token.type = svm_tok_percent;
      emit_advance(p);
      return lex_default;
    case '$':
      p->cur_token.type = svm_tok_dollar;
      emit_advance(p);
      return lex_default;
    case '[':
      p->cur_token.type = svm_tok_opbrak;
      emit_advance(p);
      return lex_default;
    case ']':
      p->cur_token.type = svm_tok_clbrak;
      emit_advance(p);
      return lex_default;
    case ' ':
    case '\t':
      ignore(p);
      return lex_default;
    case EOF:
      return NULL;
    default:
      lex_error(p, "unexpected '%c'", c);
      return NULL;
  }
  return NULL;
}

/* stateful lexer - switch considered harmful! */
int svm_lex(svm_lexer* p) {
  if(!p->source) {
    lex_error(p, "source is null");
  } else {
    if(!p->source_len) {
      p->source_len = strlen(p->source);
    }
    p->line = 1;
    p->column = 1;
    void* next_state = lex_default;
    while(next_state) {
      next_state = (*(void*(*)())next_state)(p);
    }
  }
  return !p->error;
}
