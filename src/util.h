#ifndef LILC_UTIL_H
#define LILC_UTIL_H

void die (char *file, int line, int err);

#define DIE() (die(__FILE__, __LINE__, errno))

char *read_file(char *filename);

#endif
