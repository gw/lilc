# Lilc
A hand-written compiler for a small, C-like language, targeting LLVM and built for learning purposes.

## Currently Implemented Grammar
Just for reference, as the parser is implemented by hand. Nonterminals defined in `token.h`

```text
program => block
block => expr_stmt+
expr_stmt =>
    expr SEMI
funcdef =>
    DEF ID LPAREN ID {COMMA ID} RPAREN LCURL block RCURL
call =>
    ID LPAREN (params | E) RPAREN
params =>
    params, id |
    id
if =>
    IF LPAREN expr RPAREN LCURL block RCURL elif* else?
elif =>
    ELSE IF LPAREN expr RPAREN LCURL block RCURL
else =>
    ELSE LCURL block RCURL
expr =>
    expr CMPLT term0 |
    term0            |
    call             |
    funcdef          |
    if
term0 =>
    term0 ADD term1   |
    term0 SUB term1   |
    term1
term1 =>
    term1 MUL term2 |
    term1 DIV term2 |
    term2
term2 =>
    DBL{DBL} |
    LPAREN expr RPAREN

Start Symbol: program
```

## Todo
- Mutable variables
- “Return” keyword
- Optional 'else'
- Nested if/else
    - It seems like it kinda works as is but the IR is super unreadable b/c of the way I eagerly insert blocks in `if_codegen`.
- For loop
- While loop
- Block scoping w/ corresponding semantic analysis
- Rest of comparison operators
- Allow semicolon omission from everything except for statements in user-defined blocks
- Comments (skip to \n)
- Other types, constants w/ corresponding semantic analysis, flesh out type system
- Structs
- Module system
- Screw LLVM-C, emit LLVM IR directly
- Screw LLVM IR emit x86 directly
- Screw compilers write a stack VM
