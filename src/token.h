#ifndef LILC_TOKEN_H
#define LILC_TOKEN_H

enum tok_type {
    LILC_TOK_EOS,
    LILC_TOK_SEMI,
    LILC_TOK_LPAREN,
    LILC_TOK_RPAREN,
    LILC_TOK_ADD,
    LILC_TOK_SUB,
    LILC_TOK_MUL,
    LILC_TOK_DBL,
};

struct token {
    enum tok_type cls;
    union {
      double as_dbl;
      char as_str;
    } val;
};

extern char *lilc_token_str[];

#endif