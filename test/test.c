#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "lex.h"
#include "parse.h"
#include "ast.h"
#include "util.h"

#define MAX_TOK_STR 128  // Max length of formatted token stream

static void
test_lexer(char *src_path, char *want_path) {
    char *src  = read_file(src_path);
    char *want  = read_file(want_path);

    struct lexer lex;
    lexer_init(&lex, src);

    char got[MAX_TOK_STR] = {0};
    int b = tok_strm_readf(got, &lex);

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
    lexer_init(&lex, src);
    parser_init(&parse, &lex);

    struct lilc_node_t *node;
    node = program(&parse);

    char got[MAX_NODES] = {0};
    int b = ast_readf(got, 0, node);

    assert(b < MAX_NODES);
    assert(0 == strcmp(want, got));

    free(src);
    free(want);
}

static void
test_codegen(char *src_path) {
    char *src = read_file(src_path);

    struct lexer lex;
    struct parser parse;
    lexer_init(&lex, src);
    parser_init(&parse, &lex);

    struct lilc_node_t *node;
    node = program(&parse);

    lilc_jit(node);
    // lilc_emit(node, "emitted.o");

    free(src);
}

int
main() {
    // Lexer
    test_lexer("lexer/add_2.lilc", "lexer/add_2.tok");
    test_lexer("lexer/add_sub.lilc", "lexer/add_sub.tok");

    // Parser
    test_parser("lexer/add_2.lilc", "parser/add_2.ast");
    test_parser("lexer/add_sub.lilc", "parser/add_sub.ast");

    // Codegen
    test_codegen("lexer/add_sub.lilc");

    return 0;
}

