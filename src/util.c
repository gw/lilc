#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "util.h"

void
die (char *file, int line, int err)
{
    fprintf(stderr, "%s:%d - %s\n", file, line, strerror(err));
    exit(1);
}

// Return the size of a file or -1
size_t file_size(FILE *f) {
    if (fseek(f, 0, SEEK_END)) DIE();
    size_t s = ftell(f);
    rewind(f);
    return s;
}

// Read a file into malloc'd buffer.
// Caller must free.
char *
read_file(char *filename) {
    char *buf = NULL;
    FILE *f;

    if ((f = fopen(filename, "r")) == NULL) DIE();

    // Get file length
    long len;
    if ((len = file_size(f)) == -1) DIE();

    // Allocate buffer
    if (!(buf = (char *)malloc(sizeof(char) * (len + 1)))) DIE();

    // Read file into buffer with trailing null-byte
    size_t read = fread(buf, sizeof(char), len, f);
    if (ferror(f) != 0) {
        DIE();
    } else {
        buf[read] = '\0';
    }

    fclose(f);
    return buf;
}
