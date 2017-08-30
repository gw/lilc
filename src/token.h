#ifndef LILC_TOKEN_H
#define LILC_TOKEN_H

enum tok_type {
    LILC_TOK_ERR,
    LILC_TOK_EOS,
    LILC_TOK_ID,
    LILC_TOK_DEF,
    LILC_TOK_SEMI,
    LILC_TOK_COMMA,
    LILC_TOK_LPAREN,
    LILC_TOK_RPAREN,
    LILC_TOK_LCURL,
    LILC_TOK_RCURL,
    LILC_TOK_DBL,
    LILC_TOK_ADD,
    LILC_TOK_SUB,
    LILC_TOK_MUL,
    LILC_TOK_DIV,
    LILC_TOK_CMPLT,
};

struct token {
    enum tok_type cls;
    union {
      double as_dbl;
      char *as_str;
    } val;
};

extern char *lilc_token_str[];

#endif