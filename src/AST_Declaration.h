#ifndef LANG_AST_DECLARATION_H
#define LANG_AST_DECLARATION_H

#include "Parser.h"
#include "AST_Expression.h"
#include <cstddef>

/**
    Declaration -> IDENTIFIER : TYPEIDENTIFIER { = Expression }; ?
                -> func IDENTIFIER ( ParamList ) -> TYPEIDENTIFIER ['{' Statement* '}']?
**/

// Statements can be declarations
// but declarations may also contain statements (function call)
struct AST_Statement;

struct AST_Parameter
{
    std::string name;
    data_t type;
    AST_Parameter *next;
};

struct AST_DeclType
{
    data_t type;
    data_t return_type;
    AST_Parameter *params;
};

struct AST_Declaration
{
    std::string name;
    AST_DeclType type_info;
    AST_Expression *expr;

    AST_Statement*   body;
    AST_Declaration* next;
};

bool   parse_ident_type_pair(ParserState*, Token**, data_t*);
data_t get_decl_type(ParserState*, parse_status_t*);

AST_Declaration* parse_declaration   ( ParserState*);
AST_Declaration* parse_function_decl ( ParserState*);
AST_Declaration* parse_variable_decl ( ParserState*);
AST_Parameter  * create_param_node   ( const std::string&, data_t);
AST_Declaration* create_ident_decl   ( const std::string&, data_t, AST_Expression*);
AST_Declaration* create_func_decl    ( const std::string&, data_t, AST_Parameter*, AST_Statement*);
void free_declaration(AST_Declaration*);

#endif
