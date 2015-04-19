#include "parser.h"
#include "dl_list.h"
#include "err.h"

typedef struct gen_symbol {
  char*         name;
  int           addr;
}

typedef enum {
  gen_v_reg,
  gen_v_heap,
  gen_v_const
} gen_vs;

typedef enum {
  gen_op_nop,
  gen_op_add,
  gen_op_sub,
  gen_op_mov,
  gen_op_call,
  gen_op_ret,
  gen_op_write
} gen_ops;

typedef struct gen_v {
  gen_vs     type;
}

typedef struct gen_op {
  gen_ops       action;
  char          argc;
  gen_vs[256]   argv;
}

typedef struct gen_sbc {
  gen_symbol**  symbol_table;
  dl_list*      token_stream;
  dl_list*      gen_ops;      /* one item per bytecode op */
  gen_op        cur_op;
  int           error;        /* whether or not an error has occurred */
}

/* pushes the current op to gen_ops */
void gen_sbc_emit(gen_sbc* g) {
  gen_op* nv = calloc(1, sizeof(gen_op));
  *nv = g->cur_op;
  gen_op tmp = { 0 };
  g->cur_op = tmp;
  dl_push(&g->gen_ops, nv);
}

/* next token */
svm_parser_tok* gen_sbc_next(gen_sbc* g) {

}

/* label, op or comment*/
void* gen_sbc_default(gen_sbc* g) {
  svm_parser_tok* t = gen_sbc_next(g);
  switch(t->type) {
    
  }
}

int gen_sbc_c(gen_sbc* g) {
  void* next_state = gen_sbc_default;
  while(next_state) {
    next_state = (*(void*(*)())next_state)(g);
  }
  return !g->error;
}
