#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>
#include <llvm-c/Transforms/Scalar.h>

#include "cfuhash.h"
#include "kvec.h"

#include "ast.h"
#include "codegen.h"
#include "token.h"

// Forward declaration
static LLVMValueRef
do_codegen(struct lilc_node_t *node, LLVMModuleRef module, LLVMBuilderRef builder,
           cfuhash_table_t *named_vals);


// JIT an AST and return its result
double
lilc_eval(struct lilc_node_t *node) {
    // LLVM setup
    // Contains functions and global vars. Top-level structure
    // to contain any generated IR.
    LLVMModuleRef module = LLVMModuleCreateWithName("lilc_eval");
    // Akin to a cursor--used to insert IR instructions.
    // "Methods" that take a builder instance map directly
    // to LLVM IR instructions, documented here:
    // https://llvm.org/docs/LangRef.html#llvm-language-reference-manual
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    // JIT setup
    LLVMExecutionEngineRef engine;
    LLVMLinkInMCJIT();
    char *msg;
    if(LLVMCreateExecutionEngineForModule(&engine, module, &msg) == 1) {
        fprintf(stderr, "Could not create execution engine: %s\n", msg);
        LLVMDisposeMessage(msg);
        exit(1);
    }

    // Wrap provided node in a top-level 'main' function, if the user hasn't
    // already...
    // TODO: maybe require user to provide a main function, once
    // function calls are implemented in the frontend
    if (node->type != LILC_NODE_FUNCDEF) {
        struct lilc_proto_node_t *proto = lilc_proto_node_new("main", NULL, 0);
        node = (struct lilc_node_t *)lilc_funcdef_node_new(proto, node);
    }

    // Walk AST and generate code
    // named_vals keeps track of which values are defined in the current
    // scope and what their LLVM representations are. Basically a symbol table.
    // Currently, this will only store function parameters--to be accessed when
    // generating code for a function body.
    cfuhash_table_t *named_vals = cfuhash_new_with_initial_size(64);
    LLVMValueRef val = do_codegen(node, module, builder, named_vals);
    if (!val) {
        fprintf(stderr, "\nEval failed. Exiting.\n");
        exit(1);
    }

    // Eval
    LLVMGenericValueRef eval = LLVMRunFunction(engine, val, 0, NULL);
    double result = (double)LLVMGenericValueToFloat(LLVMDoubleType(), eval);

    // Print IR
    fprintf(stderr, "Module Dump: \n");
    LLVMDumpModule(module);

    // Clean up
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

    return result;
}

// Emit a native object file at `path`, given an AST
void
lilc_emit(struct lilc_node_t *node, char *path) {
    // LLVM setup
    LLVMModuleRef module = LLVMModuleCreateWithName("lilc");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    // Wrap provided node in a top-level 'main' function, if user hasn't.
    // TODO: require user to provide a main function,
    // once function definitions are implemented in the frontend
    struct lilc_proto_node_t *proto = lilc_proto_node_new("main", NULL, 0);
    node = (struct lilc_node_t *)lilc_funcdef_node_new(proto, node);

    // Walk AST and generate code
    // named_vals keeps track of which values are defined in the current
    // scope and what their LLVM representations are. Basically a symbol table.
    // Currently, this will only store function parameters--to be accessed when
    // generating code for a function body.
    cfuhash_table_t *named_vals = cfuhash_new_with_initial_size(64);
    LLVMValueRef val = do_codegen(node, module, builder, named_vals);
    if (!val) {
        fprintf(stderr, "\nEmit Failed. Exiting.\n");
        exit(1);
    }

    // Emit a native object file to *path
    if (path == NULL) {
        fprintf(stderr, "Path for emitted file not provided\n");
        exit(1);
    }
    LLVMTargetRef target;
    LLVMBool rc;
    char *triple, *err;
    triple = LLVMGetDefaultTargetTriple();
    rc = LLVMGetTargetFromTriple(triple, &target, &err);
    if (rc) {
        fprintf(stderr, "Could not get machine target: %s", err);
        exit(1);
    }
    LLVMTargetMachineRef machine = LLVMCreateTargetMachine(
        target,
        LLVMGetDefaultTargetTriple(),
        "",  // CPU
        "",  // Features
        LLVMCodeGenLevelNone,
        LLVMRelocDefault,
        LLVMCodeModelDefault
    );
    rc = LLVMTargetMachineEmitToFile(
        machine,
        module,
        path,
        LLVMObjectFile,
        // LLVMAssemblyFile,
        &err
    );
    if (rc) {
        fprintf(stderr, "Could not emit object file: %s\n", err);
    }

    // Print IR
    fprintf(stderr, "Module Dump: \n");
    LLVMDumpModule(module);

    // Clean up
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
}

static LLVMValueRef
codegen_dbl(struct lilc_dbl_node_t *node) {
    return LLVMConstReal(LLVMDoubleType(), node->val);
}

static LLVMValueRef
codegen_var(struct lilc_var_node_t *node, cfuhash_table_t *named_vals) {
    LLVMValueRef val = cfuhash_get(named_vals, node->name);
    return val;
}

// Currently, blocks evaluate to the value of the last statement within
// them. Not sure how that will end up interacting with the 'return'
// keyword if I end up implementing that but I'll come back to it later.
// TODO: Implement block-scoping
static LLVMValueRef
codegen_block(struct lilc_block_node_t *node, LLVMModuleRef module,
              LLVMBuilderRef builder, cfuhash_table_t *named_vals) {
    LLVMValueRef val;
    for (int i = 0; i < kv_size(*node->stmts); i++) {
        val = do_codegen(kv_A(*node->stmts, i), module, builder, named_vals);
        if (!val) {
            return NULL;
        }
    }
    return val;
}

static LLVMValueRef
codegen_binop(struct lilc_bin_op_node_t *node, LLVMModuleRef module,
              LLVMBuilderRef builder, cfuhash_table_t *named_vals) {
    LLVMValueRef lhs = do_codegen(node->left, module, builder, named_vals);
    LLVMValueRef rhs = do_codegen(node->right, module, builder, named_vals);

    if(lhs == NULL || rhs == NULL) {
        return NULL;
    }

    switch(node->op) {
        case LILC_TOK_ADD: {
            // names like "addtmp" are just a hint here--
            // LLVM appends an auto-incrementing suffix if the
            // same name is assigned multiple times (SSA).
            return LLVMBuildFAdd(builder, lhs, rhs, "addtmp");
        }
        case LILC_TOK_SUB: {
            return LLVMBuildFSub(builder, lhs, rhs, "subtmp");
        }
        case LILC_TOK_MUL: {
            return LLVMBuildFMul(builder, lhs, rhs, "multmp");
        }
        case LILC_TOK_DIV: {
            return LLVMBuildFDiv(builder, lhs, rhs, "divtmp");
        }
    }
    return NULL;
}

static LLVMValueRef
codegen_proto(struct lilc_proto_node_t *node, LLVMModuleRef module,
              cfuhash_table_t *named_vals) {
    // Use an existing definition if one exists.
    LLVMValueRef func = LLVMGetNamedFunction(module, node->name);
    if(func != NULL) {
        // Verify parameter count matches.
        if(LLVMCountParams(func) != node->param_count) {
            fprintf(stderr, "Existing function exists with different parameter count\n");
            return NULL;
        }
        // Verify that the function is empty.
        if(LLVMCountBasicBlocks(func) != 0) {
            fprintf(stderr, "Existing function exists with a body\n");
            return NULL;
        }
    } else {
        // Create parameter list.
        LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef) * node->param_count);
        for (int i = 0; i < node->param_count; i++) {
            params[i] = LLVMDoubleType();  // TODO Look up types on the proto node?
        }
        // Create function type.
        LLVMTypeRef funcType = LLVMFunctionType(LLVMDoubleType(), params, node->param_count, 0);
        // Create function.
        func = LLVMAddFunction(module, node->name, funcType);
        LLVMSetLinkage(func, LLVMExternalLinkage);
    }

    // Assign parameters to named values lookup.
    for (int i = 0; i < node->param_count; i++) {
        // Not necessay, but results in more readable IR,
        // and allows for later arg lookup by name
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMSetValueName(param, node->params[i]);
        cfuhash_put(named_vals, node->params[i], param);
    }

    return func;
}

static LLVMValueRef
codegen_funcdef(struct lilc_funcdef_node_t *node, LLVMModuleRef module,
                LLVMBuilderRef builder, cfuhash_table_t *named_vals) {
    cfuhash_clear(named_vals);  // New scope

    // Codegen prototype
    LLVMValueRef func = do_codegen((struct lilc_node_t *)node->proto, module, builder, named_vals);
    if(func == NULL) {
        return NULL;
    }

    // Append a new basic block at the end of the function, named "entry"
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "entry");
    // Tell builder insert new instructions at the end of our new basic block
    LLVMPositionBuilderAtEnd(builder, block);

    // Codegen body
    LLVMValueRef body = do_codegen(node->body, module, builder, named_vals);
    if(body == NULL) {
        LLVMDeleteFunction(func);
        return NULL;
    }

    // Insert body as return value.
    LLVMBuildRet(builder, body);

    // Verify function.
    if(LLVMVerifyFunction(func, LLVMPrintMessageAction) == 1) {
        fprintf(stderr, "Invalid function\n");
        LLVMDeleteFunction(func);
        return NULL;
    }

    return func;
}

// Recursively walk an AST and generate LLVM IR
static LLVMValueRef
do_codegen(struct lilc_node_t *node, LLVMModuleRef module,
           LLVMBuilderRef builder, cfuhash_table_t *named_vals) {
    switch(node->type) {
        case LILC_NODE_DBL: {
            return codegen_dbl((struct lilc_dbl_node_t *)node);
        }
        case LILC_NODE_VAR: {
            return codegen_var((struct lilc_var_node_t *)node, named_vals);
        }
        case LILC_NODE_BLOCK: {
            return codegen_block((struct lilc_block_node_t *)node, module, builder, named_vals);
        }
        case LILC_NODE_OP_BIN: {
            return codegen_binop((struct lilc_bin_op_node_t *)node, module, builder, named_vals);
        }
        case LILC_NODE_PROTO: {
            return codegen_proto((struct lilc_proto_node_t *)node, module, named_vals);
        }
        case LILC_NODE_FUNCDEF: {
            return codegen_funcdef((struct lilc_funcdef_node_t *)node, module, builder, named_vals);
        }
    }
    return NULL;
}