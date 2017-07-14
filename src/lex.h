#ifndef LILC_LEX_H
#define LILC_LEX_H

enum token_class {
  LILC_TOK_EOS,
  LILC_TOK_ADD,
  LILC_TOK_SUB,
  LILC_TOK_INT,
};

extern char *lilc_token_str[];
int HELLO;

struct token {
  enum token_class cls;
  union {
    int as_int;
    char as_str;
  } val;
};

// Lexer state
struct lexer {
  char *source;  // Pointer to in-memory source code buffer
  char cur;      // Current character
  int offset;    // Character offset into source buffer
  struct token tok;  // Tokenized interpretation of current character
};

void lexer_init(struct lexer *lex, char *source);
int lexer_scan(struct lexer *lex);
int tok_readf(char *buf, struct lexer *lex);
int tok_strm_readf(char *buf, struct lexer *lex);

#endif
