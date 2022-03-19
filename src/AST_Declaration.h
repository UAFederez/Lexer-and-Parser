#ifndef LANG_AST_DECLARATION_H
#define LANG_AST_DECLARATION_H

#include "Parser.h"
#include "AST_Expression.h"

/**
    Declaration -> IDENTIFIER : TYPEIDENTIFIER { = Expression }; ?
                -> func IDENTIFIER ( ParamList ) -> TYPEIDENTIFIER ['{' Statement* '}']?
**/

// Statements can be declarations
// but declarations may also contain statements (function call)
struct AST_STATEMENT;

typedef struct AST_DECL_PARAM
{
    char*  name;
    data_t type;
    struct AST_DECL_PARAM *next;
} AST_Parameter;

typedef struct {
    data_t type;
    data_t return_type;
    AST_Parameter *params;
} AST_DeclType;

typedef struct
{
    char*  name;
    AST_DeclType type_info;
    AST_Expression *expr;
    struct AST_STATEMENT* body;
} AST_Declaration;

bool   parse_ident_type_pair(ParserState*, Token**, data_t*);
data_t get_decl_type(ParserState*, parse_status_t*);

AST_Declaration* parse_declaration   ( ParserState*);
AST_Declaration* parse_function_decl ( ParserState*);
AST_Declaration* parse_variable_decl ( ParserState*);
AST_Parameter  * create_param_node   ( char*, data_t);
AST_Declaration* create_ident_decl   ( char*, data_t, AST_Expression*);
AST_Declaration* create_func_decl    ( char*, data_t, AST_Parameter*, struct AST_STATEMENT*);

// TODO: deallocation functions

#endif
