#ifndef LANG_GRAPHVIZ_OUTPUT_H
#define LANG_GRAPHVIZ_OUTPUT_H

#include "AST_Declaration.h"
#include "AST_Statement.h"

typedef struct {
    FILE* out;
    int   curr_node_id;
    AST_Declaration* root;  
} GraphvizState;

int new_node(GraphvizState* gv);

int output_decl_graph(AST_Declaration*, GraphvizState* gv);
void print_graph(GraphvizState*);

#endif
