# Lilc
A hand-written compiler for a small, C-like language, targeting LLVM and built for learning purposes.

## Grammar
Just for reference, as the parser is implemented by hand. Nonterminals defined in `token.h`

```text
program => block
block => expr_stmt+
expr_stmt =>
    expr SEMI
funcdef =>
    DEF ID LPAREN ID {COMMA ID} RPAREN LCURL expr RCURL
call =>
    ID LPAREN expr{,expr} RPAREN
if =>
    IF LPAREN expr RPAREN LCURL block RCURL elif* else?
elif =>
    ELSE IF LPAREN expr RPAREN LCURL block RCURL
else =>
    ELSE LCURL block RCURL
expr =>
    expr ADD term |
    expr SUB term |
    term          |
    call          |
    funcdef       |
    if
term =>
    term MUL factor |
    term DIV factor |
    factor
factor =>
    DBL{DBL} |
    LPAREN expr RPAREN

Start Symbol: program
```