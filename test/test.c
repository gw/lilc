#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "lex.h"
#include "parse.h"
#include "ast.h"
#include "util.h"

#define MAX_TOK_STR 512  // Max length of formatted token stream

static void
test_lexer(char *src_path, char *want_path) {
    char *src  = read_file(src_path);
    char *want  = read_file(want_path);

    struct lexer l;
    lex_init(&l, src, src_path);

    char got[MAX_TOK_STR] = {0};
    // fprintf(stderr, "TOK: %s\n", got);
    int b = tok_strm_readf(got, &l);

    assert(b <= MAX_TOK_STR);
    assert(0 == strcmp(want, got));

    free(src);
    free(want);
}

#define MAX_NODES 512  // Max length of formatted AST

static void
test_parser(char *src_path, char *want_path) {
    char *src = read_file(src_path);
    char *want = read_file(want_path);

    struct lexer l;
    struct parser p;
    lex_init(&l, src, src_path);
    parser_init(&p, &l);

    struct lilc_node_t *node;
    node = parse(&p);

    char got[MAX_NODES] = {0};
    // fprintf(stderr, "AST: %s\n", got);
    int b = ast_readf(got, 0, 0, node);

    assert(b < MAX_NODES);
    assert(0 == strcmp(want, got));

    free(src);
    free(want);
}

static void
test_codegen(char *src_path, char *want_path) {
    char *src = read_file(src_path);
    char *want = read_file(want_path);

    struct lexer l;
    struct parser p;
    lex_init(&l, src, src_path);
    parser_init(&p, &l);

    struct lilc_node_t *node;
    node = parse(&p);

    double got = lilc_eval(node);
    double d_want = strtod(want, NULL);
    // fprintf(stderr, "Eval got: %f\n", got);

    double e = 0.000001;
    assert(fabs(got - d_want) < e);

    free(src);
    free(want);
}

int
main() {
    // Lexer
    test_lexer("src_examples/arith_basic.lilc", "lexer/arith_basic.tok");
    test_lexer("src_examples/arith_parens.lilc", "lexer/arith_parens.tok");
    test_lexer("src_examples/func_basic.lilc", "lexer/func_basic.tok");

    // Parser
    test_parser("src_examples/arith_basic.lilc", "parser/arith_basic.ast");
    test_parser("src_examples/arith_parens.lilc", "parser/arith_parens.ast");
    test_parser("src_examples/func_basic.lilc", "parser/func_basic.ast");
    test_parser("src_examples/if_else.lilc", "parser/if_else.ast");

    // Codegen
    test_codegen("src_examples/arith_basic.lilc", "codegen/arith_basic.result");
    test_codegen("src_examples/func_basic.lilc", "codegen/func_basic.result");
    test_codegen("src_examples/func_no_params.lilc", "codegen/func_no_params.result");
    test_codegen("src_examples/cmp_basic.lilc", "codegen/cmp_basic.result");
    test_codegen("src_examples/if_else.lilc", "codegen/if_else.result");

    return 0;
}

