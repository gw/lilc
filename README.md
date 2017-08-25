# Lilc
A hand-written compiler for a small, C-like language, targeting LLVM and built for learning purposes.

## Grammar
Just for reference, as the parser is implemented by hand. Nonterminals defined in `token.h`

```text
program => stmt+
stmt =>
    expr SEMI |
    funcdef
funcdef =>
    DEF ID LPAREN ID{,ID} RPAREN LCURL expr RCURL
call =>
    ID LPAREN expr{,expr} RPAREN
expr =>
    expr ADD term |
    expr SUB term |
    term          |
    call
term =>
    term MUL factor |
    term DIV factor |
    factor
factor =>
    DBL{DBL} |
    LPAREN expr RPAREN

Start Symbol: program
```