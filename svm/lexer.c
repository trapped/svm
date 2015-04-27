#include "lexer.h"

void* lex_ident_const(svm_lexer*);
void* lex_comment(svm_lexer*);
void* lex_default(svm_lexer*);

/* returns the next character without advancing the position */
char seek(svm_lexer* l) {
  if(l->pos+1 >= l->source_len) {
    return EOF;
  }
  return l->source[l->pos+1];
}

/* returns the next character and advances the position */
char next(svm_lexer* l) {
  char c = seek(l);
  if(c == EOF) {
    l->pos++;
    return EOF;
  }
  l->column++;
  l->pos++;
  return c;
}

/* goes back a character */
char previous(svm_lexer* l) {
  if(l->pos-1 < 0) {
    return EOF;
  }
  l->column--;
  l->pos--;
  return l->source[l->pos];
}

/* ignores the next character */
void ignore(svm_lexer* l) {
  l->cur_token.start_pos++;
}

/* token type to string */
char* svm_tok_str(svm_lexer_tok* t) {
  static char* types[] = { "unknown", "newline", "period", "comma", "colon",
    "equal", "percent", "dollar", "opbrak", "clbrak", "ident", "const",
    "comment" };
  return types[t->type];
}

/* prints a token to stderr */
void svm_tok_print(svm_lexer* l, svm_lexer_tok* t) {
  fprintf(stderr, "%s:%d-%d(%d): '%.*s'\n", svm_tok_str(t), t->start_pos,
    t->end_pos, t->end_pos - t->start_pos,
    t->type == svm_tok_newline ? 2 : t->end_pos - t->start_pos,
    t->type == svm_tok_newline ? "\\n" : &l->source[t->start_pos]);
}

/* moves cur_token to heap and pushes it to token_stream */
void emit(svm_lexer* l) {
  l->cur_token.end_pos = l->pos;
  l->cur_token.line = l->line;
  l->cur_token.column =
    l->column - (l->cur_token.end_pos - l->cur_token.start_pos);
  svm_lexer_tok* nv = calloc(1, sizeof(svm_lexer_tok));
  *nv = l->cur_token;
  /* it appears that you can't reset using a compound literal directly */
  svm_lexer_tok tmp = { 0, .start_pos = l->pos, .end_pos = l->pos };
  l->cur_token = tmp;
  dl_push(&l->token_stream, nv);
  //svm_tok_print(l, nv);
}

/* emits and advances position */
void emit_advance(svm_lexer* l) {
  next(l);
  emit(l);
  previous(l);
}

#define lex_error(l, fmt, ...) do {       \
  l->error = 1;                           \
  fprintf(stderr, "%s:%d:%d: " fmt "\n",  \
    l->filename, l->line, l->column,      \
    ##__VA_ARGS__);                       \
} while(0)
//backtrace_symbols_fd(1, 1, stdout); only on GNU/Linux?

/* identifiers or constants */
void* lex_ident_const(svm_lexer* l) {
  char c;
  switch(c = next(l)) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':
      if(l->cur_token.type == svm_tok_const) {
        /* a constant outside quotes is a number, which can't contain
           letters */
        l->cur_token.type = svm_tok_ident;
      } else if(l->cur_token.type == svm_tok_unknown) {
        /* if it starts with a letter it's for sure an ident */
        l->cur_token.type = svm_tok_ident;
      }
      /* fallthrough */
    case '0' ... '9':
      if(l->cur_token.type == svm_tok_unknown) {
        /* holds as long as there are no letters */
        l->cur_token.type = svm_tok_const;
      }
      /* fallthrough */
      return lex_ident_const;
    case EOF:
      lex_error(l, "unexpected EOF");
      return NULL;
    default:
      emit(l);
      previous(l); /* we haven't processed it after all */
      return lex_default;
  }
}

/* comments */
void* lex_comment(svm_lexer* l) {
  switch(next(l)) {
    case '\n':
      emit(l);
      previous(l); /* let the \n be lexed */
      return lex_default;
    default:
      return lex_comment;
  }
}

/* anything (mostly dispatches) */
void* lex_default(svm_lexer* l) {
  char c;
  switch(c = next(l)) {
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '0' ... '9':
    case '_':
      previous(l);
      return lex_ident_const;
    case '#':
      l->cur_token.type = svm_tok_comment;
      /* ignore hash character */
      ignore(l);
      return lex_comment;
    case '\n':
      l->line++;
      l->column = 0;
      l->cur_token.type = svm_tok_newline;
      emit_advance(l);
      return lex_default;
    case '\r':
      ignore(l);
      return lex_default;
    case '.':
      l->cur_token.type = svm_tok_period;
      emit_advance(l);
      return lex_default;
    case ':':
      l->cur_token.type = svm_tok_colon;
      emit_advance(l);
      return lex_default;
    case '=':
      l->cur_token.type = svm_tok_equal;
      emit_advance(l);
      return lex_default;
    case ',':
      l->cur_token.type = svm_tok_comma;
      emit_advance(l);
      return lex_default;
    case '%':
      l->cur_token.type = svm_tok_percent;
      emit_advance(l);
      return lex_default;
    case '$':
      l->cur_token.type = svm_tok_dollar;
      emit_advance(l);
      return lex_default;
    case '[':
      l->cur_token.type = svm_tok_opbrak;
      emit_advance(l);
      return lex_default;
    case ']':
      l->cur_token.type = svm_tok_clbrak;
      emit_advance(l);
      return lex_default;
    case ' ':
    case '\t':
      ignore(l);
      return lex_default;
    case EOF:
      return NULL;
    default:
      lex_error(l, "unexpected '%c'", c);
      return NULL;
  }
  return NULL;
}

/* stateful lexer - switch considered harmful! */
int svm_lex(svm_lexer* l) {
  if(!l->source) {
    lex_error(l, "source is null");
  } else {
    if(!l->source_len) {
      l->source_len = strlen(l->source);
    }
    l->line = 1;
    l->column = 1;
    void* next_state = lex_default;
    while(next_state) {
      next_state = (*(void*(*)())next_state)(l);
    }
  }
  return !l->error;
}
