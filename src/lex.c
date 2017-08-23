#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "token.h"

static enum tok_type
set_tok_type(struct lexer *lex, enum tok_type t) {
    return lex->tok.cls = t;
}

static double
set_tok_val_dbl(struct lexer *lex, double val) {
    return lex->tok.val.as_dbl = val;
}

static char *
set_tok_val_str(struct lexer *lex, char *val) {
    return lex->tok.val.as_str = strdup(val);
}


static void
lexer_putback(struct lexer *lex) {
    lex->offset--;
}

#define MAX_IDENT 64
// Tokenize an entire identifier.
// Identifiers must be <= 64 char long and can contain
// alphas, nums, and '_', but must start with an alpha.
static enum tok_type
consume_id(struct lexer *lex, char c) {
    char buf[MAX_IDENT];
    int i = 0;
    do {
        buf[i++] = c;
    } while (
        (isalpha(c = lex->source[lex->offset++]) || isdigit(c) || c == '_') &&
        i < MAX_IDENT - 1
    );
    buf[i] = '\0';
    lexer_putback(lex);

    set_tok_val_str(lex, buf);
    if (strcmp(buf, "def") == 0) {
        return set_tok_type(lex, LILC_TOK_DEF);
    } else {
        return set_tok_type(lex, LILC_TOK_ID);
    }
}

// Tokenize an entire number.
// Only floats supported for now
static enum tok_type
consume_number(struct lexer *lex, char c) {
    double n = 0;
    do {
        n = n * 10 + (c - '0');
    } while (isdigit(c = lex->source[lex->offset++]));
    lexer_putback(lex);

    set_tok_val_dbl(lex, n);
    return set_tok_type(lex, LILC_TOK_DBL);
}

// Scan the next token in the source input.
// Returns the class of the scanned token, dies on error.
enum tok_type
lexer_scan(struct lexer *lex) {
    char c;
    while (1) {
        c = lex->source[lex->offset++];
        switch (c) {
            case ' ':
            case '\n':  // TODO incr line number for debugging
            case '\t': continue;
            case ';': return set_tok_type(lex, LILC_TOK_SEMI);
            case '(': return set_tok_type(lex, LILC_TOK_LPAREN);
            case ')': return set_tok_type(lex, LILC_TOK_RPAREN);
            case '+': return set_tok_type(lex, LILC_TOK_ADD);
            case '-': return set_tok_type(lex, LILC_TOK_SUB);
            case '*': return set_tok_type(lex, LILC_TOK_MUL);
            case '/': return set_tok_type(lex, LILC_TOK_DIV);
            case '\0': return set_tok_type(lex, LILC_TOK_EOS);
            default:
                if (isalpha(c)) return consume_id(lex, c);
                if (isdigit(c)) return consume_number(lex, c);
                fprintf(stderr, "Unrecognized input char: '%c'\n", c);
                exit(1);
        }
    }
}

// Assert current token type.
// If assertion passes, returns 1 and advances lexer
// Otherwise returns 0 and does not adavnce lexer
int
lexer_advance(struct lexer *lex, enum tok_type t) {
    if (lex->tok.cls == t) {
        lexer_scan(lex);
        return 1;
    } else {
        return 0;
    }
}

// Read a formatted version of the remaining token stream into a buffer,
// returning the number of bytes written.
// Not bothering with safe, bounded buffer wrting, as I'd just use a dynamic
// array instead.
int
tok_strm_readf(char *buf, struct lexer *lex) {
    int i = 0;
    while (lexer_scan(lex)) {
        i += sprintf(buf + i, "<");
        switch (lex->tok.cls) {
            case LILC_TOK_DBL:
                i += sprintf(buf + i, "%s,", lilc_token_str[lex->tok.cls]);
                i += sprintf(buf + i, "%.1f", lex->tok.val.as_dbl);
                break;
            case LILC_TOK_ID:
                i += sprintf(buf + i, "%s,", lilc_token_str[lex->tok.cls]);
                i += sprintf(buf + i, "%s", lex->tok.val.as_str);
                break;
            default:
                i += sprintf(buf + i, "%s", lilc_token_str[lex->tok.cls]);
                break;
        }
        i += sprintf(buf + i, ">");
    }
    return i;
}

void
lexer_init(struct lexer *lex, char *source) {
    lex->source = source;
    lex->offset = 0;
}
