#ifndef LANG_AST_EXPRESSION_H
#define LANG_AST_EXPRESSION_H

#include "Parser.h"

typedef enum { 
    TYPE_INT , TYPE_FLOAT , TYPE_FUNCTION, TYPE_VOID
} data_t;

typedef enum { 
    EXPR_ADD  , EXPR_SUB   , EXPR_MUL    , EXPR_DIV   ,
    EXPR_CALL , EXPR_IDENT , EXPR_INT    , EXPR_FLOAT ,
    EXPR_ARG  , EXPR_NEG   , EXPR_ASSIGN
} expr_t;

typedef struct AST_EXPR
{
    expr_t           type;
    struct AST_EXPR  *lhs;
    struct AST_EXPR  *rhs;

    union {
        char*  ident_name;
        long   int_value;
        double flt_value;
    };
} AST_Expression;


AST_Expression* create_expr      ( expr_t, AST_Expression*, AST_Expression*);
AST_Expression* create_expr_ident( const char*, AST_Expression* lhs, AST_Expression* rhs );
AST_Expression* create_expr_int  ( long );
AST_Expression* create_expr_float( double );

AST_Expression* parse_atom       ( ParserState*);
AST_Expression* parse_expression ( ParserState*, size_t);
AST_Expression* maybe_parse_call ( ParserState* );
void free_expression ( AST_Expression*);
bool check_if_binary_op(ParserState*, Operator_Info*);

#endif
