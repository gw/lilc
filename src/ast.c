#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lex.h"

/*
 * Human-readable strings for each node class
 */
char *lilc_node_str[] = {
  [LILC_NODE_PROTO] = "proto",
  [LILC_NODE_FUNCDEF] = "funcdef",
};


struct lilc_dbl_node_t *
lilc_dbl_node_new(double val) {
    struct lilc_dbl_node_t *node = malloc(sizeof(struct lilc_dbl_node_t));
    node->base.type = LILC_NODE_DBL;
    node->val = val;
    return node;
};

struct lilc_bin_op_node_t *
lilc_bin_op_node_new(struct lilc_node_t *left, struct lilc_node_t *right, enum tok_type op) {
    struct lilc_bin_op_node_t *node = malloc(sizeof(struct lilc_bin_op_node_t));
    node->base.type = LILC_NODE_OP_BIN;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
};

// TODO add a types array arg
struct lilc_proto_node_t *
lilc_proto_node_new(char *name, char **args, unsigned int arg_count) {
    struct lilc_proto_node_t *node = malloc(sizeof(struct lilc_proto_node_t));
    node->base.type = LILC_NODE_PROTO;
    node->name = strdup(name);
    node->arg_count = arg_count;

    // Copy arguments.
    node->args = malloc(sizeof(char*) * arg_count);
    for(int i=0; i<arg_count; i++) {
        node->args[i] = strdup(args[i]);
    }

    return node;
};

struct lilc_funcdef_node_t *
lilc_funcdef_node_new(struct lilc_proto_node_t *proto, struct lilc_node_t *body) {
    struct lilc_funcdef_node_t *node = malloc(sizeof(struct lilc_funcdef_node_t));
    node->base.type = LILC_NODE_FUNCDEF;
    node->proto = proto;
    node->body = body;
    return node;
}

// Read a formatted version of an AST into a buffer, returning the number of
// bytes written.
int
ast_readf(char *buf, int i, struct lilc_node_t *node) {
    i += sprintf(buf + i, "(");
    switch (node->type) {
        case LILC_NODE_DBL:
            i += sprintf(buf + i, "dbl ");
            i += sprintf(buf + i, "%.1f", ((struct lilc_dbl_node_t *)node)->val);
            break;
        case LILC_NODE_OP_BIN:
            i += sprintf(buf + i, "%s ", lilc_token_str[((struct lilc_bin_op_node_t *)node)->op]);
            i = ast_readf(buf, i, ((struct lilc_bin_op_node_t *)node)->left);
            i = ast_readf(buf, i, ((struct lilc_bin_op_node_t *)node)->right);
            break;
        case LILC_NODE_FUNCDEF:
            i += sprintf(buf + i, "%s ", lilc_node_str[((struct lilc_funcdef_node_t *)node)->base.type]);
            i = ast_readf(buf, i, ((struct lilc_funcdef_node_t *)node)->proto);
            i = ast_readf(buf, i, ((struct lilc_funcdef_node_t *)node)->body);
            break;
        case LILC_NODE_PROTO:
            i += sprintf(buf + i, "%s: ", lilc_node_str[((struct lilc_proto_node_t *)node)->base.type]);
            i += sprintf(buf + i, "%s ", ((struct lilc_proto_node_t *)node)->name);
            i += sprintf(buf + i, "%d", ((struct lilc_proto_node_t *)node)->arg_count);
            break;
        default:
            i += sprintf(buf + i, "Unknown: %d", node->type);
    }
    i += sprintf(buf + i, ")");
    return i;
}