#ifndef SVM_LEXER_H
#define SVM_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <execinfo.h>
#include "dl_list.h"
#include "err.h"

typedef enum {
  svm_tok_unknown,
  svm_tok_newline,  /* \r\n */
  svm_tok_period,   /* .    */
  svm_tok_comma,    /* ,    */
  svm_tok_colon,    /* :    */
  svm_tok_equal,    /* =    */
  svm_tok_percent,  /* %    */
  svm_tok_dollar,   /* $    */
  svm_tok_opbrak,   /* [    */
  svm_tok_clbrak,   /* ]    */
  svm_tok_ident,    /* func */
  svm_tok_const,    /* 1 "" */
  svm_tok_comment   /* #abc */
} svm_tok_type;

typedef struct {
  svm_tok_type  type;
  int           start_pos;
  int           end_pos;
  /* value is sliced from source text */
} svm_lexer_tok;

typedef struct svm_lexer {
  int             source_len;   /* save source length to save cycles */
  char*           source;       /* full source text (at pos 0)       */
  int             pos;          /* current position                  */
  char*           filename;
  int             line;
  int             column;
  svm_lexer_tok   cur_token;
  dl_list*        token_stream; /* double linked list for tokens     */
  void*           tok_emit_cb;  /* emit() callback                   */
  int             error;        /* whether an error has occurred     */
} svm_lexer;

void  svm_tok_print(svm_lexer*, svm_lexer_tok*);
int   svm_lex(svm_lexer*);

#endif
