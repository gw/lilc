#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvec.h"

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

struct lilc_block_node_t *
lilc_block_node_new(void) {
    struct lilc_block_node_t *node = malloc(sizeof(struct lilc_block_node_t));
    node->base.type = LILC_NODE_BLOCK;
    node->stmts = (node_vec_t *)malloc(sizeof(node_vec_t));
    kv_init(*node->stmts);
    return node;
};

struct lilc_var_node_t *
lilc_var_node_new(char *name) {
    struct lilc_var_node_t *node = malloc(sizeof(struct lilc_var_node_t));
    node->base.type = LILC_NODE_VAR;
    node->name = name;
    return node;
}

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
lilc_proto_node_new(char *name, char **params, unsigned int param_count) {
    struct lilc_proto_node_t *node = malloc(sizeof(struct lilc_proto_node_t));
    node->base.type = LILC_NODE_PROTO;
    node->name = name;
    node->param_count = param_count;

    // Copy params pointer array to heap
    node->params = malloc(sizeof(char*) * param_count);
    for(int i = 0; i < param_count; i++) {
        node->params[i] = params[i];
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

struct lilc_funccall_node_t *
lilc_funccall_node_new(char *name, struct lilc_node_t **args, unsigned int arg_count) {
    struct lilc_funccall_node_t *node = malloc(sizeof(struct lilc_funccall_node_t));
    node->base.type = LILC_NODE_FUNCCALL;
    node->name = name;

    // Copy args pointer array to heap
    // TODO: refactor to an AST union type, so we can easily
    // always allocate enough space for the largest possible
    // node here, instead of hardcoding bin_op_node here just
    // cuz it's big
    node->args = malloc(sizeof(struct lilc_bin_op_node_t) * arg_count);
    for(int i = 0; i < arg_count; i++) {
        node->args[i] = args[i];
    }

    node->arg_count = arg_count;
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
        case LILC_NODE_BLOCK: {
            struct lilc_block_node_t *n = (struct lilc_block_node_t *)node;
            i += sprintf(buf + i, "block\n");
            for (int j = 0; j < kv_size(*n->stmts); j++) {
                i = ast_readf(buf, i, kv_A(*n->stmts, j));
                i += sprintf(buf + i, "\n");
            }
            break;
        }
        case LILC_NODE_VAR:
            i += sprintf(buf + i, "var ");
            i += sprintf(buf + i, "%s", ((struct lilc_var_node_t *)node)->name);
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
        case LILC_NODE_PROTO: {
            struct lilc_proto_node_t *n = (struct lilc_proto_node_t *)node;
            i += sprintf(buf + i, "%s", n->name);
            // Param list
            i += sprintf(buf + i, "[");
            if (n->param_count > 0) {
                for (int j = 0; j < n->param_count; j++) {
                    i += sprintf(buf + i, "%s,", n->params[j]);
                }
                i--;  // Delete trailing comma
            }
            i += sprintf(buf + i, "]");
            break;
        }
        case LILC_NODE_FUNCCALL: {
            struct lilc_funccall_node_t *n = (struct lilc_funccall_node_t *)node;
            i += sprintf(buf + i, "call %s ", n->name);
            for (int j = 0; j < n->arg_count; j++) {
                i = ast_readf(buf, i, n->args[j]);
            }
            break;
        }
        default:
            i += sprintf(buf + i, "Unknown: %d", node->type);
    }
    i += sprintf(buf + i, ")");
    return i;
}