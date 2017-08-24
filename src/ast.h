#ifndef LILC_AST_H
#define LILC_AST_H

#include "token.h"

enum node_type {
    LILC_NODE_DBL,
    LILC_NODE_VAR,
    LILC_NODE_EXPR,
    LILC_NODE_OP_BIN,
    LILC_NODE_PROTO,
    LILC_NODE_FUNCDEF,
};

/*
 * Containers for each node type
 */

// Base data shared by all nodes
struct lilc_node_t {
    enum node_type type;
};

// Double immediate node
struct lilc_dbl_node_t {
    struct lilc_node_t base;
    double val;
};

// Variable expression node
struct lilc_var_node_t {
    struct lilc_node_t base;
    char *name;
};

// Binary operation node
struct lilc_bin_op_node_t {
    struct lilc_node_t base;
    struct lilc_node_t *left;
    struct lilc_node_t *right;
    enum tok_type op;
};

// Function prototype node
struct lilc_proto_node_t {
    struct lilc_node_t base;
    char *name;
    char **params;
    unsigned int param_count;
};

// Function declaration node
struct lilc_funcdef_node_t {
    struct lilc_node_t base;
    struct lilc_proto_node_t *proto;
    struct lilc_node_t *body;
};

/*
 *Constructors
 */
struct lilc_dbl_node_t *
lilc_dbl_node_new(double val);

struct lilc_var_node_t *
lilc_var_node_new(char *name);

struct lilc_bin_op_node_t *
lilc_bin_op_node_new(struct lilc_node_t *left, struct lilc_node_t *right, enum tok_type op);

struct lilc_proto_node_t *
lilc_proto_node_new(char *name, char **params, unsigned int param_count);

struct lilc_funcdef_node_t *
lilc_funcdef_node_new(struct lilc_proto_node_t *proto, struct lilc_node_t *body);

int
ast_readf(char *buf, int i, struct lilc_node_t *node);

#endif