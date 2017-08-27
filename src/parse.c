#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvec.h"

#include "ast.h"
#include "lex.h"
#include "parse.h"
#include "token.h"

// Interface for parsing-related functionality for each token type.
struct vtable {
    int lbp;  // Left Binding Power
    struct lilc_node_t *(*as_prefix)(struct parser *p, struct token t);
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

// double
static struct lilc_node_t *
dbl_prefix(struct parser *p, struct token t) {
    return (struct lilc_node_t *)lilc_dbl_node_new(t.val.as_dbl);
}

// identifier
static struct lilc_node_t *
id_prefix(struct parser *p, struct token t) {
    return (struct lilc_node_t *)lilc_var_node_new(t.val.as_str);
}

// Parenthesized arithmetic expressions
static struct lilc_node_t *
lparen_prefix(struct parser *p, struct token t) {
    // 0 rbp here b/c we obviously want to
    // continue parsing the contents of the
    // parenthesized expression. Parenthesized
    // expressions are always subexpressions of
    // any containing expression they're part of.
    struct lilc_node_t *node = expression(p, 0);
    lexer_consumef(p->lex, LILC_TOK_RPAREN);
    return node;
}

#define MAX_FUNC_PARAMS 16

// Function calls
static struct lilc_node_t *
lparen_infix(struct parser *p, struct token t, struct lilc_node_t *left) {
    struct lilc_node_t *args[MAX_FUNC_PARAMS];
    unsigned int arg_count = 0;

    do {
        args[arg_count++] = expression(p, 0);
    } while (lexer_consume(p->lex, LILC_TOK_COMMA));

    lexer_consumef(p->lex, LILC_TOK_RPAREN);

    char *name = ((struct lilc_var_node_t *)left)->name;
    return (struct lilc_node_t *)lilc_funccall_node_new(name, args, arg_count);
}

// "+", "-", "*", "/"
static struct lilc_node_t *
bin_op_infix(struct parser *p, struct token t, struct lilc_node_t *left) {
    return (struct lilc_node_t *)lilc_bin_op_node_new(left, expression(p, vtables[t.cls].lbp), t.cls);
}

// "def"
static struct lilc_node_t *
funcdef_prefix(struct parser *p, struct token t) {
    char *name = p->lex->tok.val.as_str;
    lexer_consumef(p->lex, LILC_TOK_ID);

    lexer_consumef(p->lex, LILC_TOK_LPAREN);

    char *params[MAX_FUNC_PARAMS];
    unsigned int param_count = 0;
    while (p->lex->tok.cls != LILC_TOK_RPAREN && param_count <= MAX_FUNC_PARAMS) {
        if (p->lex->tok.cls != LILC_TOK_ID) {
            fprintf(stderr, "Expected id, got %s", lilc_token_str[p->lex->tok.cls]);
            exit(1);
        }
        params[param_count++] = p->lex->tok.val.as_str;
        lexer_scan(p->lex);
        lexer_consume(p->lex, LILC_TOK_COMMA);
    }

    if (param_count == MAX_FUNC_PARAMS) {
        fprintf(stderr, "Too many params for function %s\n", name);
        exit(1);
    }

    lexer_consumef(p->lex, LILC_TOK_RPAREN);
    lexer_consumef(p->lex, LILC_TOK_LCURL);

    struct lilc_node_t *body = expression(p, 0);

    lexer_consumef(p->lex, LILC_TOK_RCURL);

    struct lilc_proto_node_t *proto = lilc_proto_node_new(name, params, param_count);
    return (struct lilc_node_t *)lilc_funcdef_node_new(proto, body);
}

// Lookup array for token vtable implementations
struct vtable vtables[] = {
    [LILC_TOK_EOS] = {
        .lbp = 0,
    },
    [LILC_TOK_SEMI] = {
        .lbp = 0,
    },
    [LILC_TOK_DEF] = {
        .lbp = 0,
        .as_prefix = funcdef_prefix,
    },
    [LILC_TOK_LPAREN] = {
        // If this had a 0 lbp, then when parsing a function call,
        // the identifier would get parsed as its own expression,
        // i.e. `expression()` would return before continuing to parse
        // the LPAREN, whereas we want the entire function call to be
        // treated as a single expression.
        .lbp = 10,
        .as_prefix = lparen_prefix,
        .as_infix = lparen_infix,
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
    [LILC_TOK_DIV] = {
        .lbp = 2,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_DBL] = {
        .as_prefix = dbl_prefix,
    },
    [LILC_TOK_ID] = {
        .as_prefix = id_prefix,
    },
};

// Main loop of Pratt (Top-Down Operator Precedence) expression parsing.
// `rbp`: right binding power
static struct lilc_node_t *
expression(struct parser *p, int rbp) {
    struct token t;
    struct lilc_node_t *left;

    t = p->lex->tok;
    lexer_scan(p->lex);
    left = vtables[t.cls].as_prefix(p, t);

    // Precedence climbing! Any expression on the right side
    // of an operator with a higher binding power is considered
    // a subexpression, so we want to continue parsing it!
    while (rbp < vtables[p->lex->tok.cls].lbp) {
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
    struct lilc_block_node_t *block = lilc_block_node_new();

    lexer_scan(p->lex);  // Load first token
    while (p->lex->tok.cls != LILC_TOK_EOS) {
        node = stmt(p);
        kv_push(struct lilc_node_t *, *block->stmts, node);
        lexer_consumef(p->lex, LILC_TOK_SEMI);
    }

    // TODO check if EOS or error and respond appropriately
    return block;
}

void
parser_init(struct parser *p, struct lexer *lex) {
    p->lex = lex;
}