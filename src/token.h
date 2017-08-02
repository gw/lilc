#ifndef LILC_TOKEN_H
#define LILC_TOKEN_H

enum tok_type {
    LILC_TOK_EOS,
    LILC_TOK_SEMI,
    LILC_TOK_ADD,
    LILC_TOK_SUB,
    LILC_TOK_INT,
};

struct token {
    enum tok_type cls;
    union {
      int as_int;
      char as_str;
    } val;
};

extern char *lilc_token_str[];

#endif