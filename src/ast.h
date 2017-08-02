#ifndef LILC_AST_H
#define LILC_AST_H

#include "token.h"

enum node_type {
    LILC_NODE_INT,
    LILC_NODE_EXPR,
    LILC_NODE_OP_BIN,
};

/*
 * Containers for each node type
 */

// Base data shared by all nodes
struct lilc_node_t {
    enum node_type type;
};

// Integer immediate node
struct lilc_int_node_t {
    struct lilc_node_t base;
    int val;
};

// Binary operation node
struct lilc_bin_op_node_t {
    struct lilc_node_t base;
    struct lilc_node_t *left;
    struct lilc_node_t *right;
    enum tok_type op;
};

/*
 *Constructors
 */
struct lilc_int_node_t *
lilc_int_node_new(int val);

struct lilc_bin_op_node_t *
lilc_bin_op_node_new(struct lilc_node_t *left, struct lilc_node_t *right, enum tok_type op);

int
ast_readf(char *buf, int i, struct lilc_node_t *node);

#endif