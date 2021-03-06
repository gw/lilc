#include "token.h"

/*
 * Human-readable strings for each token class
 */
char *lilc_token_str[] = {
  [LILC_TOK_EOS] = "end-of-source",
  [LILC_TOK_ID] = "id",
  [LILC_TOK_DEF] = "def",
  [LILC_TOK_SEMI] = ";",
  [LILC_TOK_COMMA] = ",",
  [LILC_TOK_LPAREN] = "(",
  [LILC_TOK_RPAREN] = ")",
  [LILC_TOK_LCURL] = "{",
  [LILC_TOK_RCURL] = "}",
  [LILC_TOK_DBL] = "dbl",
  [LILC_TOK_ADD] = "+",
  [LILC_TOK_SUB] = "-",
  [LILC_TOK_MUL] = "*",
  [LILC_TOK_DIV] = "/",
  [LILC_TOK_CMPLT] = "<",
  [LILC_TOK_IF] = "if",
  [LILC_TOK_ELSE] = "else",
};
