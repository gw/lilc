#ifndef LILC_AST_H
#define LILC_AST_H

#include "kvec.h"

#include "token.h"

enum node_type {
    LILC_NODE_DBL,
    LILC_NODE_BLOCK,
    LILC_NODE_VAR,
    LILC_NODE_OP_BIN,
    LILC_NODE_PROTO,
    LILC_NODE_FUNCDEF,
    LILC_NODE_FUNCCALL,
    LILC_NODE_IF,
};

/*
 * Dynamic array of AST nodes
 */
typedef kvec_t(struct lilc_node_t *) lilc_node_vec_t;
lilc_node_vec_t *lilc_node_vec_new(void);
#define lilc_node_vec_push(vec, node) kv_push(struct lilc_node_t *, vec, node)

/*
 * Containers for each node type
 */

// Base data shared by all nodes
struct lilc_node_t {
    enum node_type type;
};

// Block node
struct lilc_block_node_t {
    struct lilc_node_t base;
    lilc_node_vec_t *stmts;
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

// Function call node
struct lilc_funccall_node_t {
    struct lilc_node_t base;
    char *name;
    struct lilc_node_t **args;
    unsigned int arg_count;
};

// If / else if / else node
struct lilc_if_node_t {
    struct lilc_node_t base;
    struct lilc_node_t *cond;
    struct lilc_block_node_t *then_block;
    struct lilc_block_node_t *else_block;
};

// Union struct--ends up being the width of the largest
// member. Only used in places where you need to allocate
// space for an AST node whose type you don't know in advance
// (e.g. funccall_node_new) and thus need to allocate enough
// space to fit the largest possible node type.
union lilc_ast_union_t {
    struct lilc_block_node_t a;
    struct lilc_dbl_node_t b;
    struct lilc_var_node_t c;
    struct lilc_bin_op_node_t d;
    struct lilc_proto_node_t e;
    struct lilc_funcdef_node_t f;
    struct lilc_funccall_node_t g;
};

/*
 *Constructors
 */
struct lilc_dbl_node_t *
lilc_dbl_node_new(double val);

struct lilc_block_node_t *
lilc_block_node_new(lilc_node_vec_t *stmts);

struct lilc_var_node_t *
lilc_var_node_new(char *name);

struct lilc_bin_op_node_t *
lilc_bin_op_node_new(struct lilc_node_t *left, struct lilc_node_t *right, enum tok_type op);

struct lilc_proto_node_t *
lilc_proto_node_new(char *name, char **params, unsigned int param_count);

struct lilc_funcdef_node_t *
lilc_funcdef_node_new(struct lilc_proto_node_t *proto, struct lilc_node_t *body);

struct lilc_funccall_node_t *
lilc_funccall_node_new(char *name, struct lilc_node_t **args, unsigned int arg_count);

struct lilc_if_node_t *
lilc_if_node_new(struct lilc_node_t *cond, struct lilc_block_node_t *then_block);

int
ast_readf(char *buf, int i, struct lilc_node_t *node);

#endif