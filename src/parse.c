#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvec.h"

#include "ast.h"
#include "lex.h"
#include "parse.h"
#include "token.h"
#include "util.h"

static void *
err(struct parser *p, char *msg) {
    p->err = msg;
    return NULL;
}

void
parser_init(struct parser *p, struct lexer *l) {
    p->lex = l;
}

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
static struct lilc_node_t * block(struct parser *p);

/*
 * `as_prefix` and `as_infix` (`nud` and `led` respectively in Pratt's lingo)
 * implementations for each token class
 * TODO: Implement error checking, otherwise it'll SEGFAULT on bad input
 */

/*
DBL
*/
static struct lilc_node_t *
dbl_prefix(struct parser *p, struct token t) {
    return (struct lilc_node_t *)lilc_dbl_node_new(t.val.as_dbl);
}

/*
ID
*/
static struct lilc_node_t *
id_prefix(struct parser *p, struct token t) {
    return (struct lilc_node_t *)lilc_var_node_new(t.val.as_str);
}

/*
if => IF LPAREN expr RPAREN LCURL block RCURL elif* else?
*/
static struct lilc_node_t *
if_prefix(struct parser *p, struct token t) {
    lex_consumef(p->lex, LILC_TOK_LPAREN);

    struct lilc_node_t *cond = expression(p, 0);

    lex_consumef(p->lex, LILC_TOK_RPAREN);
    lex_consumef(p->lex, LILC_TOK_LCURL);

    struct lilc_block_node_t *then_block = (struct lilc_block_node_t *)block(p);

    lex_consumef(p->lex, LILC_TOK_RCURL);

    struct lilc_if_node_t *node = lilc_if_node_new(cond, then_block);
    if (!node) {
        return err(p, "if: Could not allocate 'if' node\n");
    }

    if (lex_consume(p->lex, LILC_TOK_ELSE)) {
        lex_consumef(p->lex, LILC_TOK_LCURL);

        struct lilc_block_node_t *else_block = (struct lilc_block_node_t *)block(p);
        if (!(node->else_block = (struct lilc_block_node_t *)block(p))) {
            return err(p, "if: Could not parse 'else' block\n");
        }
        node->else_block = else_block;

        lex_consumef(p->lex, LILC_TOK_RCURL);
    }

    return (struct lilc_node_t *)node;
}

/*
factor => LPAREN expr RPAREN
*/
static struct lilc_node_t *
lparen_prefix(struct parser *p, struct token t) {
    // 0 rbp here b/c we obviously want to
    // continue parsing the contents of the
    // parenthesized expression. Parenthesized
    // expressions are always subexpressions of
    // any containing expression they're part of.
    struct lilc_node_t *node = expression(p, 0);
    lex_consumef(p->lex, LILC_TOK_RPAREN);
    return node;
}

#define MAX_FUNC_PARAMS 16


/*
call => ID LPAREN (params | E) RPAREN
*/
static struct lilc_node_t *
lparen_infix(struct parser *p, struct token t, struct lilc_node_t *left) {
    struct lilc_node_t *args[MAX_FUNC_PARAMS];
    struct lilc_node_t *arg;
    unsigned int arg_count = 0;

    while (!lex_is(p->lex, LILC_TOK_RPAREN) && arg_count <= MAX_FUNC_PARAMS) {
        if (!(arg = expression(p, 0))) {
            return err(p, "Could not parse call argument\n");
        }

        args[arg_count++] = arg;

        lex_consume(p->lex, LILC_TOK_COMMA);
    }

    lex_consumef(p->lex, LILC_TOK_RPAREN);

    char *name = ((struct lilc_var_node_t *)left)->name;
    return (struct lilc_node_t *)lilc_funccall_node_new(name, args, arg_count);
}

/*
expr =>
    expr CMPLT term0 |
    term0            |
term0 =>
    term0 ADD term1   |
    term0 SUB term1   |
    term1
term1 =>
    term1 MUL term2 |
    term1 DIV term2 |
    term2
*/
static struct lilc_node_t *
bin_op_infix(struct parser *p, struct token t, struct lilc_node_t *left) {
    struct lilc_node_t *right = expression(p, vtables[t.cls].lbp);

    if (!right) {
        return err(p, "Could not parse right operand of binary expr\n");
    }

    return (struct lilc_node_t *)lilc_bin_op_node_new(left, right, t.cls);
}


/*
funcdef => DEF ID LPAREN ID {COMMA ID} RPAREN LCURL block RCURL
*/
static struct lilc_node_t *
funcdef_prefix(struct parser *p, struct token t) {
    char *funcname = p->lex->tok.val.as_str;

    lex_consumef(p->lex, LILC_TOK_ID);
    lex_consumef(p->lex, LILC_TOK_LPAREN);

    // Parse parameter list
    char *params[MAX_FUNC_PARAMS];
    unsigned int param_count = 0;
    while (!lex_is(p->lex, LILC_TOK_RPAREN) && param_count <= MAX_FUNC_PARAMS) {
        if (!lex_is(p->lex, LILC_TOK_ID)) {
            return err(p, "funcdef params: Expected identifier\n");
        }

        params[param_count++] = p->lex->tok.val.as_str;

        lex_scan(p->lex);
        lex_consume(p->lex, LILC_TOK_COMMA);
    }

    if (param_count == MAX_FUNC_PARAMS) {
        return err(p, "funcdef: Too many parameters\n");
    }

    lex_consumef(p->lex, LILC_TOK_RPAREN);
    lex_consumef(p->lex, LILC_TOK_LCURL);

    struct lilc_node_t *body = block(p);

    lex_consumef(p->lex, LILC_TOK_RCURL);

    struct lilc_proto_node_t *proto = lilc_proto_node_new(funcname, params, param_count);
    return (struct lilc_node_t *)lilc_funcdef_node_new(proto, body);
}

// Lookup array for token vtable implementations
// Note that prefix-only operators don't need a
// binding power--prefix functions are always called.
struct vtable vtables[] = {
    // Constants
    [LILC_TOK_DBL] = {
        .as_prefix = dbl_prefix,
    },
    // Keywords
    [LILC_TOK_DEF] = {
        .as_prefix = funcdef_prefix,
    },
    [LILC_TOK_ID] = {
        .as_prefix = id_prefix,
    },
    [LILC_TOK_IF] = {
        .as_prefix = if_prefix,
    },
    // Operators
    [LILC_TOK_CMPLT] = {
        .lbp = 1,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_ADD] = {
        .lbp = 2,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_SUB] = {
        .lbp = 2,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_MUL] = {
        .lbp = 3,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_DIV] = {
        .lbp = 3,
        .as_infix = bin_op_infix,
    },
    [LILC_TOK_LPAREN] = {
        // If this had a 0 lbp, then when parsing a function call,
        // the identifier would get parsed as its own expression,
        // i.e. `expression()` would return before continuing to parse
        // the LPAREN, whereas we want the entire function call to be
        // treated as a single expression.
        .lbp = 10,
        .as_prefix = lparen_prefix,  // Parenthesized expressions
        .as_infix = lparen_infix,    // Function calls
    },
};

// Main loop of Pratt (Top-Down Operator Precedence) expression parsing.
// `rbp`: right binding power
static struct lilc_node_t *
expression(struct parser *p, int rbp) {
    struct token t;
    struct lilc_node_t *left;

    t = p->lex->tok;
    lex_scan(p->lex);

    if (!vtables[t.cls].as_prefix) {
        return err(p, "expression: No prefix function found\n");
    }
    left = vtables[t.cls].as_prefix(p, t);

    // Precedence climbing! Any expression on the right side
    // of an operator with a higher binding power is considered
    // a subexpression, so we want to continue parsing it!
    while (rbp < vtables[p->lex->tok.cls].lbp) {
        t = p->lex->tok;
        lex_scan(p->lex);

        if (!vtables[t.cls].as_infix) {
            return err(p, "expression: No infix function found\n");
        }
        left = vtables[t.cls].as_infix(p, t, left);
    }

    return left;
}


/*
expr_stmt => expr SEMI
*/
static struct lilc_node_t *
expr_stmt(struct parser *p) {
    struct lilc_node_t *node = expression(p, 0);
    lex_consumef(p->lex, LILC_TOK_SEMI);
    return node;
}


/*
block => expr_stmt+
*/
static struct lilc_node_t *
block(struct parser *p) {
    lilc_node_vec_t *stmts = lilc_node_vec_new();
    struct lilc_node_t *node;

    // If this is the top-level block that represents the series of expr_stmts
    // that constitute the whole program, it won't be surrounded by curlies.
    // If it's a sub-block, it will be.
    while (!lex_is(p->lex, LILC_TOK_EOS) && !lex_is(p->lex, LILC_TOK_RCURL)) {
        if (!(node = expr_stmt(p))) return NULL;
        lilc_node_vec_push(*stmts, node);
    }

    return (struct lilc_node_t *)lilc_block_node_new(stmts);
}

/*
program => block
*/
static struct lilc_node_t *
program(struct parser *p) {
    lex_scan(p->lex);  // Load first token
    return block(p);

}

// Parse a Lilc program. Returns a pointer to the root node
// upon successful parsing--otherwise crashes.
struct lilc_node_t *
parse(struct parser *p) {
    struct lilc_node_t *root;
    if (!(root = program(p))) {
        die(p->lex->filename, p->lex->lineno, p->err);
    }
    return root;
}
