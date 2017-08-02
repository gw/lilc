#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lex.h"

struct lilc_int_node_t *
lilc_int_node_new(int val) {
    struct lilc_int_node_t *node = malloc(sizeof(struct lilc_int_node_t));
    node->base.type = LILC_NODE_INT;
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

// Read a formatted version of an AST into a buffer, returning the number of
// bytes written.
int
ast_readf(char *buf, int i, struct lilc_node_t *node) {
    i += sprintf(buf + i, "(");
    switch (node->type) {
        case LILC_NODE_INT:
            i += sprintf(buf + i, "int ");
            i += sprintf(buf + i, "%d", ((struct lilc_int_node_t *)node)->val);
            break;
        case LILC_NODE_OP_BIN:
            i += sprintf(buf + i, "%s ", lilc_token_str[((struct lilc_bin_op_node_t *)node)->op]);
            i = ast_readf(buf, i, ((struct lilc_bin_op_node_t *)node)->left);
            i = ast_readf(buf, i, ((struct lilc_bin_op_node_t *)node)->right);
            break;
        default:
            i += sprintf(buf + i, "Unknown: %d", node->type);
    }
    i += sprintf(buf + i, ")");
    return i;
}