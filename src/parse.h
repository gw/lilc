#ifndef LILC_PARSE_H
#define LILC_PARSE_H

#include "ast.h"
#include "lex.h"
#include "token.h"

struct parser {
    struct lexer *lex;
    char *err;
};

struct lilc_node_t *
parse(struct parser *p);

void
parser_init(struct parser *parse, struct lexer *l);

#endif