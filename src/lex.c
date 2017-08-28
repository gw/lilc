#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "token.h"

static enum tok_type
set_tok_type(struct lexer *l, enum tok_type t) {
    return l->tok.cls = t;
}

static void
lex_putback(struct lexer *l) {
    l->offset--;
}

#define MAX_IDENT 64
// Tokenize an entire identifier.
// Identifiers must be <= 64 char long and can contain
// alphas, nums, and '_', but must start with an alpha.
static enum tok_type
consume_id(struct lexer *l, char c) {
    char buf[MAX_IDENT];
    int i = 0;

    do {
        buf[i++] = c;
    } while (
        (isalpha(c = l->source[l->offset++]) || isdigit(c) || c == '_') &&
        i < MAX_IDENT - 1
    );
    buf[i] = '\0';
    lex_putback(l);

    if (strcmp(buf, "def") == 0) {
        return set_tok_type(l, LILC_TOK_DEF);
    } else {
        // Non-keyword identifier
        l->tok.val.as_str = strdup(buf);
        return set_tok_type(l, LILC_TOK_ID);
    }
}

// Tokenize an entire number.
// Only floats supported for now
static enum tok_type
consume_number(struct lexer *l, char c) {
    double n = 0;
    do {
        n = n * 10 + (c - '0');
    } while (isdigit(c = l->source[l->offset++]));
    lex_putback(l);

    l->tok.val.as_dbl = n;
    return set_tok_type(l, LILC_TOK_DBL);
}

// Scan the next token in the source input.
// Returns the class of the scanned token, dies on error.
enum tok_type
lex_scan(struct lexer *l) {
    char c;
    while (1) {
        c = l->source[l->offset++];
        switch (c) {
            case ' ':
            case '\n':  // TODO incr line number for debugging
            case '\t': continue;
            case ',': return set_tok_type(l, LILC_TOK_COMMA);
            case ';': return set_tok_type(l, LILC_TOK_SEMI);
            case '(': return set_tok_type(l, LILC_TOK_LPAREN);
            case ')': return set_tok_type(l, LILC_TOK_RPAREN);
            case '{': return set_tok_type(l, LILC_TOK_LCURL);
            case '}': return set_tok_type(l, LILC_TOK_RCURL);
            case '+': return set_tok_type(l, LILC_TOK_ADD);
            case '-': return set_tok_type(l, LILC_TOK_SUB);
            case '*': return set_tok_type(l, LILC_TOK_MUL);
            case '/': return set_tok_type(l, LILC_TOK_DIV);
            case '\0': return set_tok_type(l, LILC_TOK_EOS);
            default:
                if (isalpha(c)) return consume_id(l, c);
                if (isdigit(c)) return consume_number(l, c);
                fprintf(stderr, "Unrecognized input char: '%c'\n", c);
                exit(1);
        }
    }
}

// Return 1 if current token is type `t`,
// 0 otherwise.
int
lex_is(struct lexer *l, enum tok_type t) {
    return l->tok.cls == t;
}

// Assert current token type.
// If assertion passes, returns 1 and advances lexer,
// otherwise returns 0 and does not advance lexer.
int
lex_consume(struct lexer *l, enum tok_type t) {
    if (lex_is(l, t)) {
        lex_scan(l);
        return 1;
    } else {
        return 0;
    }
}

// Assert current token type, failing hard.
// If assertion passes, returns 1 and advances lexer,
// otherwise prints an error message and dies.
int
lex_consumef(struct lexer *l, enum tok_type t) {
    if (!lex_consume(l, t)) {
        fprintf(
            stderr,
            "Expected token '%s', got '%s'\n",
            lilc_token_str[t], lilc_token_str[l->tok.cls]
        );
        exit(1);
    }
    return 1;
}

// Read a formatted version of the remaining token stream into a buffer,
// returning the number of bytes written.
// Not bothering with safe, bounded buffer wrting, as I'd just use a dynamic
// array instead.
int
tok_strm_readf(char *buf, struct lexer *l) {
    int i = 0;
    while (lex_scan(l)) {
        i += sprintf(buf + i, "<");
        switch (l->tok.cls) {
            case LILC_TOK_DBL:
                i += sprintf(buf + i, "%s,", lilc_token_str[l->tok.cls]);
                i += sprintf(buf + i, "%.1f", l->tok.val.as_dbl);
                break;
            case LILC_TOK_ID:
                i += sprintf(buf + i, "%s,", lilc_token_str[l->tok.cls]);
                i += sprintf(buf + i, "%s", l->tok.val.as_str);
                break;
            default:
                i += sprintf(buf + i, "%s", lilc_token_str[l->tok.cls]);
                break;
        }
        i += sprintf(buf + i, ">");
    }
    return i;
}

void
lex_init(struct lexer *l, char *source) {
    l->source = source;
    l->offset = 0;
}
