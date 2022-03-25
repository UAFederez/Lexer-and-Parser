#include "GraphvizOutput.h"
#include <stdio.h>
#include <string.h>

const char* TYPE_STR[] = { "TYPE_INT", "TYPE_FLOAT", "TYPE_FUNCTION", "TYPE_VOID" };
const char* STMT_TYPE_STR[] = { "STMT_EXPR", "STMT_DECL", "STMT_IF", "STMT_FOR", "STMT_RETURN" };

const char* NAME_FMT   = "    str_%d[label=\"{\\\"%s\\\"}\"];\n";
const char* ENUM_FMT   = "    enum_%d[label=\"{%s}\"];\n";
const char* DECL_FMT   = "    decl_%d[label=\"{AST_Declaration | {<f1>name |<f2> type_info|<f3> expr|<f4> body|<f5>next}}\"];\n";
const char* TYPE_FMT   = "    type_%d[label=\"{AST_DeclType|{<f1>type|<f2>return_type|<f3>params}}\"];\n";
const char* PARAM_FMT  = "    param_%d[label=\"{AST_Parameter|{<f1>name|<f2>type|<f3>next}}\"];\n";
const char* EXPR_FMT   = "    expr_%d[label=\"{%s|{<f1>lhs|<f2>rhs}}\"];\n";
const char* EXPR_FMT_I = "    expr_%d[label=\"{%ld|{<f1>lhs|<f2>rhs}}\"];\n";
const char* EXPR_FMT_F = "    expr_%d[label=\"{%d|{<f1>lhs|<f2>rhs}}\"];\n";
const char* STMT_FMT   = "    stmt_%d[label=\"{AST_Statement|{<f1>type|<f2>expr|<f3>decl|<f4>body|<f5>else_blk|<f6>next}}\"];\n";

void print_graph(GraphvizState* gv)
{
    fprintf(gv->out, "digraph G {\n");
    fprintf(gv->out, "    node[shape=record fontname=Arial];\n");
    output_decl_graph(gv->root, gv);
    fprintf(gv->out, "\n}\n");
}

int output_expression(AST_Expression* expr, GraphvizState* gv)
{
    if (expr != NULL)
    {
        int expr_id = new_node(gv);
        switch(expr->type)
        {
            case EXPR_ADD       : fprintf(gv->out, EXPR_FMT, expr_id, "+")               ; break ;
            case EXPR_DIV       : fprintf(gv->out, EXPR_FMT, expr_id, "/")               ; break ;
            case EXPR_MUL       : fprintf(gv->out, EXPR_FMT, expr_id, "*")               ; break ;
            case EXPR_SUB       : fprintf(gv->out, EXPR_FMT, expr_id, "-")               ; break ;
            case EXPR_CALL      : fprintf(gv->out, EXPR_FMT, expr_id, "EXPR_CALL")       ; break ;
            case EXPR_ARG       : fprintf(gv->out, EXPR_FMT, expr_id, "EXPR_ARG")        ; break ;
            case EXPR_COMP_LESS : fprintf(gv->out, EXPR_FMT, expr_id, "\\<")             ; break ;
            case EXPR_IDENT     : fprintf(gv->out, EXPR_FMT, expr_id, expr->ident_name.c_str())  ; break ;
            case EXPR_FLOAT     : fprintf(gv->out, EXPR_FMT_F, expr_id, expr->flt_value) ; break ;
            case EXPR_INT       : fprintf(gv->out, EXPR_FMT_I, expr_id, expr->int_value) ; break ;
            default             : fprintf(gv->out, EXPR_FMT, expr_id, "op")              ; break ;
        }
        
        int lhs_id = output_expression(expr->lhs, gv);
        int rhs_id = output_expression(expr->rhs, gv);

        if(lhs_id != -1) fprintf(gv->out, "    expr_%d:<f1> -> expr_%d;\n", expr_id, lhs_id);
        if(rhs_id != -1) fprintf(gv->out, "    expr_%d:<f2> -> expr_%d;\n", expr_id, rhs_id);
        return expr_id;
    }
    return -1;
}

int output_statement(AST_Statement* stmt, GraphvizState* gv)
{
    if (stmt != NULL)
    {
        int stmt_id = new_node(gv);
        int type_id = new_node(gv);

        fprintf(gv->out, STMT_FMT, stmt_id);
        fprintf(gv->out, ENUM_FMT, type_id, STMT_TYPE_STR[stmt->type]);

        int expr_id = output_expression(stmt->expr, gv);
        int decl_id = output_decl_graph(stmt->decl, gv);
        int body_id = output_statement(stmt->body, gv);
        int else_id = output_statement(stmt->else_blk, gv);
        int next_id = output_statement(stmt->next, gv);
        
        fprintf(gv->out, "    stmt_%d:<f1> -> enum_%d;\n", stmt_id, type_id);
        if(expr_id != -1) fprintf(gv->out, "    stmt_%d:<f2> -> expr_%d;\n", stmt_id, expr_id);
        if(decl_id != -1) fprintf(gv->out, "    stmt_%d:<f3> -> decl_%d;\n", stmt_id, decl_id);
        if(body_id != -1) fprintf(gv->out, "    stmt_%d:<f4> -> stmt_%d;\n", stmt_id, body_id);
        if(else_id != -1) fprintf(gv->out, "    stmt_%d:<f5> -> stmt_%d;\n", stmt_id, else_id);
        if(next_id != -1) fprintf(gv->out, "    stmt_%d:<f6> -> stmt_%d;\n", stmt_id, next_id);
        return stmt_id;
    }
    return -1;
}

// Name, Info, Type, Expr body
int output_decl_type_params(AST_Parameter* params, GraphvizState* gv)
{
    if (params != NULL)
    {
        int param_id = new_node(gv);
        int name_id  = new_node(gv);
        int type_id  = new_node(gv);

        fprintf(gv->out, PARAM_FMT, param_id);
        fprintf(gv->out, NAME_FMT, name_id, params->name.c_str());
        fprintf(gv->out, ENUM_FMT, type_id, TYPE_STR[params->type]);

        if (params->next != NULL)
        {
            int next_param_id = output_decl_type_params(params->next, gv);
            fprintf(gv->out, "    param_%d:<f3> -> param_%d;\n", param_id, next_param_id);
        }

        fprintf(gv->out, "    param_%d:<f1> -> str_%d;\n", param_id, name_id);
        fprintf(gv->out, "    param_%d:<f2> -> enum_%d;\n", param_id, type_id);
        return param_id;
    }
    return -1;
}

int output_decl_type_info(AST_DeclType* type_info, GraphvizState* gv)
{
    int info_id = new_node(gv);
    int info_type_id = new_node(gv);
    int info_ret_id  = new_node(gv);
               
    fprintf(gv->out, TYPE_FMT, info_id);
    fprintf(gv->out, ENUM_FMT, info_type_id, TYPE_STR[type_info->type]);
    fprintf(gv->out, ENUM_FMT, info_ret_id, TYPE_STR[type_info->return_type]);

    fprintf(gv->out, "    type_%d:<f1> -> enum_%d;\n", info_id, info_type_id);
    fprintf(gv->out, "    type_%d:<f2> -> enum_%d;\n", info_id, info_ret_id);

    if (type_info->params != NULL)
    {
        int params_id = output_decl_type_params(type_info->params, gv);
        fprintf(gv->out, "    type_%d:<f3> -> param_%d;\n", info_id, params_id);
    }
    return info_id;
}

int output_decl_graph(AST_Declaration* root, GraphvizState* gv)
{
    if (root != NULL)
    {
        int decl_id = new_node(gv);
        int name_id = new_node(gv);

        fprintf(gv->out, DECL_FMT, decl_id);
        fprintf(gv->out, NAME_FMT, name_id, root->name.c_str());

        int type_info_id = output_decl_type_info(&root->type_info, gv);
        int expr_id      = output_expression(root->expr, gv);
        int body_id      = output_statement (root->body, gv);
        int next_id      = output_decl_graph(root->next, gv);

        // Edges
        fprintf(gv->out, "    decl_%d:<f1> -> str_%d;\n", decl_id, name_id);
        fprintf(gv->out, "    decl_%d:<f2> -> type_%d;\n", decl_id, type_info_id);

        if (expr_id != -1) fprintf(gv->out, "    decl_%d:<f3> -> expr_%d;\n", decl_id, expr_id);
        if (body_id != -1) fprintf(gv->out, "    decl_%d:<f4> -> stmt_%d;\n", decl_id, body_id);
        if (next_id != -1) fprintf(gv->out, "    decl_%d:<f5> -> decl_%d;\n", decl_id, next_id);

        return decl_id;
    }
    return -1;
}

int new_node(GraphvizState* gv)
{
    return gv->curr_node_id++;
}
