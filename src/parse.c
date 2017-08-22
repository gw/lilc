#include "ast.h"
#include "lex.h"
#include "parse.h"
#include "token.h"

// Interface for parsing-related functionality for each token type.
struct vtable {
    int lbp;  // Left Binding Power
    struct lilc_node_t *(*as_prefix)(struct token t);
    struct lilc_node_t *(*as_infix)(struct parser *p, struct token t, struct lilc_node_t *left);
    void (*repr)(void);
};

// Forward declarations
struct vtable vtables[];
static struct lilc_node_t *expression(struct parser *p, int rbp);

/*
 * `as_prefix` and `as_infix` (`nud` and `led` respectively in Pratt's lingo)
 * implementations for each token class
 * TODO: Implement error checking, otherwise it'll SEGFAULT on bad input
 */

// int
static struct lilc_node_t *int_prefix(struct token t) {
    return (struct lilc_node_t *)lilc_int_node_new(t.val.as_int);
}

// "+", "-", "*", "/"
static struct lilc_node_t *bin_op_infix(struct parser *p, struct token t, struct lilc_node_t *left) {
    return (struct lilc_node_t *)lilc_bin_op_node_new(left, expression(p, vtables[t.cls].lbp), t.cls);
}

// Lookup array for token vtable implementations
struct vtable vtables[] = {
    [LILC_TOK_EOS] = {
        .lbp = 0,
    },
    [LILC_TOK_SEMI] = {
        .lbp = 0,
    },
    [LILC_TOK_ADD] = {
        .lbp = 1,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_SUB] = {
        .lbp = 1,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_MUL] = {
        .lbp = 2,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_INT] = {
        .as_prefix = int_prefix,
    },
};

// Main loop of Pratt (Top-Down Operator Precedence) expression parsing.
// `rbp`: right binding power
static struct lilc_node_t *
expression(struct parser *p, int rbp) {
    struct token t;
    struct lilc_node_t *left;

    t = p->lex->tok;
    left = vtables[t.cls].as_prefix(t);

    lexer_scan(p->lex);

    while (rbp < vtables[p->lex->tok.cls].lbp) {  // "precedence climbing"!
        t = p->lex->tok;
        lexer_scan(p->lex);
        left = vtables[t.cls].as_infix(p, t, left);
    }
    return left;
}

// expr;
static struct lilc_node_t *
stmt(struct parser *p) {
    return expression(p, 0);
}

// stmt+
struct lilc_node_t *
program(struct parser *p) {
    struct lilc_node_t *node;
    while (lexer_scan(p->lex) > 0) {  // Call program parser function
        node = stmt(p);
    }
    // TODO check if EOS or error and respond appropriately
    return node;
}

void
parser_init(struct parser *p, struct lexer *lex) {
    p->lex = lex;
}