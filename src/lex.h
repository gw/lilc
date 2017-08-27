#ifndef LILC_LEX_H
#define LILC_LEX_H

#include "token.h"

// Lexer state
struct lexer {
    char *source;     // Pointer to in-memory source code buffer
    char cur;         // Current character
    int offset;       // Character offset into source buffer
    struct token tok; // Tokenized interpretation of current character
};

void lex_init(struct lexer *l, char *source);
int lex_consume(struct lexer *l, enum tok_type want);
int lex_consumef(struct lexer *l, enum tok_type want);
enum tok_type lex_scan(struct lexer *l);
int tok_strm_readf(char *buf, struct lexer *l);

#endif
