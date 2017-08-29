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

    struct lexer lex;
    lex_init(&lex, src, src_path);

    char got[MAX_TOK_STR] = {0};
    int b = tok_strm_readf(got, &lex);
    // fprintf(stderr, "TOK: %s\n", got);

    assert(b <= MAX_TOK_STR);
    assert(0 == strcmp(want, got));

    free(src);
    free(want);
}

#define MAX_NODES 128  // Max length of formatted AST

static void
test_parser(char *src_path, char *want_path) {
    char *src = read_file(src_path);
    char *want = read_file(want_path);

    struct lexer lex;
    struct parser parse;
    lex_init(&lex, src, src_path);
    parser_init(&parse, &lex);

    struct lilc_node_t *node;
    node = program(&parse);

    char got[MAX_NODES] = {0};
    int b = ast_readf(got, 0, node);
    // fprintf(stderr, "AST: %s\n", got);

    assert(b < MAX_NODES);
    assert(0 == strcmp(want, got));

    free(src);
    free(want);
}

static void
test_codegen(char *src_path, char *want_path) {
    char *src = read_file(src_path);
    char *want = read_file(want_path);

    struct lexer lex;
    struct parser parse;
    lex_init(&lex, src, src_path);
    parser_init(&parse, &lex);

    struct lilc_node_t *node;
    node = program(&parse);

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

    // Codegen
    test_codegen("src_examples/arith_basic.lilc", "codegen/arith_basic.result");
    test_codegen("src_examples/func_basic.lilc", "codegen/func_basic.result");
    test_codegen("src_examples/func_no_params.lilc", "codegen/func_no_params.result");

    return 0;
}

