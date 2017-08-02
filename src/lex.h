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

void lexer_init(struct lexer *lex, char *source);
enum tok_type lexer_scan(struct lexer *lex);
int tok_strm_readf(char *buf, struct lexer *lex);

#endif
