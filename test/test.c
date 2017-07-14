#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lex.h"
#include "util.h"

static void
test_lexer(const char *src_path, const char *want_path) {
    char *src  = read_file(src_path);
    char *want  = read_file(want_path);

    struct lexer lex;
    lexer_init(&lex, src);

    char got[64] = {0};
    int b = tok_strm_readf(got, &lex);

    assert(0 == strcmp(want, got));
}

int
main() {
    // Run tests
    test_lexer("lexer/add_2.lilc", "lexer/add_2.out");
}

