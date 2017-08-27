#ifndef LILC_PARSE_H
#define LILC_PARSE_H

#include "ast.h"
#include "lex.h"
#include "token.h"

// Parser state. Thin wrapper around the lexer, may end up containing some extra
// info for debugging, but not super useful right now.
struct parser {
    struct lexer *lex;
};

struct lilc_node_t *
program(struct parser *);

void
parser_init(struct parser *parse, struct lexer *l);

#endif