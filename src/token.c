#include "token.h"

/*
 * Human-readable strings for each token class
 */
char *lilc_token_str[] = {
  [LILC_TOK_EOS] = "end-of-source",
  [LILC_TOK_SEMI] = ";",
  [LILC_TOK_ADD] = "+",
  [LILC_TOK_SUB] = "-",
  [LILC_TOK_INT] = "int",
};
