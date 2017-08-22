#ifndef LILC_CODEGEN_H
#define LILC_CODEGEN_H

#include "ast.h"

void
lilc_codegen(struct lilc_node_t *node, char *path, void *result);

int
lilc_eval(struct lilc_node_t *node);

void
lilc_emit(struct lilc_node_t *node, char *path);

#endif
