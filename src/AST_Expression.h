#ifndef LANG_AST_EXPRESSION_H
#define LANG_AST_EXPRESSION_H

#include "Parser.h"
#include <cstddef>

enum data_t 
{ 
    TYPE_INT , TYPE_FLOAT , TYPE_FUNCTION, TYPE_VOID
};

enum expr_t 
{ 
    EXPR_ADD  , EXPR_SUB   , EXPR_MUL    , EXPR_DIV       ,
    EXPR_CALL , EXPR_IDENT , EXPR_INT    , EXPR_FLOAT     ,
    EXPR_ARG  , EXPR_NEG   , EXPR_ASSIGN , EXPR_COMP_LESS ,
};

struct AST_Expression
{
    expr_t          type;
    AST_Expression *lhs;
    AST_Expression *rhs;

    std::string ident_name;
    long   int_value;
    double flt_value;
};


AST_Expression* create_expr      ( expr_t, AST_Expression*, AST_Expression*);
AST_Expression* create_expr_ident( const std::string&, AST_Expression* lhs, AST_Expression* rhs );
AST_Expression* create_expr_int  ( long );
AST_Expression* create_expr_float( double );

AST_Expression* parse_atom       ( ParserState*);
AST_Expression* parse_expression ( ParserState*, size_t);
AST_Expression* maybe_parse_call ( ParserState* );
bool check_if_binary_op(ParserState*, Operator_Info*);
void free_expression(AST_Expression*);
#endif
