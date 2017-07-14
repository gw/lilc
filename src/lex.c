#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lex.h"

// Human-readable strings for each token class
char *lilc_token_str[] = {
  [LILC_TOK_EOS] = "end-of-source",
  [LILC_TOK_ADD] = "+",
  [LILC_TOK_SUB] = "-",
  [LILC_TOK_INT] = "int",
};

void lexer_init(struct lexer *lex, char *source) {
    lex->source = source;
    lex->offset = 0;
}

// Set the type of the current token
static int set_tok_type(struct lexer *lex, enum token_class tok_type) {
    return lex->tok.cls = tok_type;
};

static int consume_number(struct lexer *lex, char c) {
    int n = 0;
    do {
        n = n * 10 + (c - '0');
    } while (isdigit(c = lex->source[lex->offset++]));
    lex->offset--;  // put back

    lex->tok.val.as_int = n;
    return set_tok_type(lex, LILC_TOK_INT);
};

// Scan the next token in the source input
// Returns 0 on EOS, -1 on error
int lexer_scan(struct lexer *lex) {
    char c;

loop:
    switch (c = lex->source[lex->offset++]) {
        case ' ':
        case '\n':  // TODO incr line number for debugging
        case '\t': goto loop;
        case '+': return set_tok_type(lex, LILC_TOK_ADD);
        case '-': return set_tok_type(lex, LILC_TOK_SUB);
        case '\0': return set_tok_type(lex, LILC_TOK_EOS);
        default:
            if (isdigit(c)) return consume_number(lex, c);
            // Unrecognized input character
            fprintf(stderr, "Unrecognized input char: %d\n", c);
            return -1;
    }
};

// Read a formatted version of the current token into
// a buffer, returning the number of bytes written or -1
// TODO: Should probably use snprintf, unless we
// restrict how we print tok.val.
int tok_readf(char *buf, struct lexer *lex) {
    int i = 0;

    i += sprintf(buf + i, "<");

    switch (lex->tok.cls) {
        case LILC_TOK_INT:
            // Token has a meaningful value
            i += sprintf(buf + i, "%s,", lilc_token_str[lex->tok.cls]);
            i += sprintf(buf + i, "%d", lex->tok.val.as_int);
            break;
        default:
            i += sprintf(buf + i, "%s", lilc_token_str[lex->tok.cls]);
            break;
    }

    i += sprintf(buf + i, ">");
    return i;
};

#define MAX_TOK_STR 64  // Max length of formatted token stream

// Read a formatted version of the entire token stream
// into a buffer, returning the number of bytes written
int
tok_strm_readf(char *buf, struct lexer *lex) {
    int i = 0;

    while (i <= MAX_TOK_STR && lexer_scan(lex)) {
        i += tok_readf(buf + i, lex);
    }

    return i;
};

