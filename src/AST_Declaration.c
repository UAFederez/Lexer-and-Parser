#include "AST_Declaration.h"
#include "AST_Statement.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

AST_Declaration* parse_declaration(ParserState* parser)
{
    AST_Declaration* decl = NULL;

    if (match_token(parser, KEYWORD_FUNC))
        decl = parse_function_decl(parser);
    else
        decl = parse_variable_decl(parser);

    return decl;
}

AST_Declaration* parse_function_decl(ParserState* parser)
{
    get_next_token(parser);

    Token* func_id = parser->curr_token;
    if (!match_token(parser, TOKEN_IDENTIFIER))
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        return NULL;
    }
    get_next_token(parser);

    // Parse an optional parameter list, which is a list of AST_Declaration's but
    // those that are not themselves function declarations
    if (!match_token(parser, TOKEN_LEFT_PAREN))
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        return NULL;
    }
    get_next_token(parser);

    bool expect_another_param = false;
    AST_Parameter*  params = NULL;
    AST_Parameter** curr_param = &params;

    while(!match_token(parser, TOKEN_RIGHT_PAREN) || expect_another_param)
    {
        Token* param_ident = NULL;
        data_t param_type  = TYPE_VOID;

        if (!parse_ident_type_pair(parser, &param_ident, &param_type))
        {
            parser->status = PARSE_ERR_INVALID_PARAM;
            return NULL;
        }

        if (*curr_param == NULL)
            *curr_param = create_param_node(param_ident->lexeme, param_type);
        curr_param = &((*curr_param)->next);

        if ((expect_another_param = match_token(parser, TOKEN_COMMA)))
            get_next_token(parser);
    }

    // The loop above may have ended because the param-type pair was invalid
    if (!match_token(parser, TOKEN_RIGHT_PAREN) && !expect_another_param)
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        return NULL;
    }
    get_next_token(parser);

    if (!match_token(parser, TOKEN_ARROW))
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        return NULL;
    }
    get_next_token(parser);
    
    parse_status_t stat = PARSE_SUCCESS;
    data_t return_type  = get_decl_type(parser, &stat);

    // TODO: for now, forward declarations of functions or lone prototypes are
    // not yet permitted
    AST_Statement* body = parse_block(parser, true);

    if (stat != PARSE_SUCCESS)
    {
        parser->status     = PARSE_ERR_INVALID_TYPE;
        parser->curr_token = func_id;   // valid token to get line number from
        return NULL;
    }
    return create_func_decl(func_id->lexeme, return_type, params, body);
}

AST_Declaration* parse_variable_decl(ParserState* parser)
{
    // Used for identifier declarations i.e: IDENTIFIER : IDENTIFIER {= Expression}?;
    Token* ident_token = NULL;
    data_t decl_type   = TYPE_VOID;

    if (!parse_ident_type_pair(parser, &ident_token, &decl_type))
        return NULL;

    // Parse optional initialization value
    bool will_expect_expr    = false; 
    AST_Expression* init_val = NULL;

    if (match_token(parser, TOKEN_OP_EQU))
    {
        get_next_token(parser);
        init_val = parse_expression(parser, 0);
        will_expect_expr = true;
    }

    if (init_val == NULL && will_expect_expr)
        return NULL;

    return create_ident_decl(ident_token->lexeme, decl_type, init_val);
}

bool parse_ident_type_pair(ParserState* parser, Token** out_ident, data_t* out_type)
{
    Token* start_token = parser->curr_token; // for rolling back advances on curr_token

    if (!match_token(parser, TOKEN_IDENTIFIER))
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        return false;
    }

    // Parse the first token, which should be an indentifier, but not
    // an identifier referring to a primitive type
    parse_status_t stat = PARSE_SUCCESS;
    get_decl_type(parser, &stat);

    if (stat == PARSE_SUCCESS)
    {
        parser->status = PARSE_ERR_INVALID_DECL;
        parser->curr_token = start_token;
        return false;
    }

    *out_ident = parser->curr_token;
    get_next_token(parser);

    if (!match_token(parser, TOKEN_COLON))
    {
        parser->status     = PARSE_ERR_INVALID_PARAM;
        parser->curr_token = start_token;
        return false;
    }
    get_next_token(parser); // consume ':'

    if (!match_token(parser, TOKEN_IDENTIFIER))
    {
        parser->status     = PARSE_ERR_INVALID_TYPE;
        parser->curr_token = start_token;
        return false;
    }

    stat = PARSE_SUCCESS;
    *out_type = get_decl_type(parser, &stat);

    if (stat != PARSE_SUCCESS)
    {
        parser->status = PARSE_ERR_INVALID_TYPE;
        parser->curr_token = start_token;
        return false;
    }
    get_next_token(parser);
    return true;
}

data_t get_decl_type(ParserState* parser, parse_status_t* status)
{
    static const char* PRIMITIVE_TYPE_NAMES[] = { "int", "float" };

    if (!match_token(parser, TOKEN_IDENTIFIER))
    {
        *status = PARSE_ERR_INVALID_TYPE;
        return TYPE_VOID;
    }

    data_t type = TYPE_VOID;
    bool has_matched_type = false;
    for (size_t i = 0; i < 2 && !has_matched_type; i++) 
    {
        if (strcmp(parser->curr_token->lexeme, PRIMITIVE_TYPE_NAMES[i]) == 0)
        {
            type = TYPE_INT + i;
            has_matched_type = true;
        }
    }

    if (!has_matched_type)
        *status = PARSE_ERR_INVALID_TYPE;

    return type;
}

AST_Declaration* create_func_decl(char* name, data_t return_type, AST_Parameter* params, AST_Statement* stmts)
{
    AST_Declaration* func = (AST_Declaration*) malloc(sizeof(AST_Declaration));
    func->expr   = NULL;

    func->type_info.params      = params;
    func->type_info.type        = TYPE_FUNCTION;
    func->type_info.return_type = return_type;
    func->body = stmts;

    size_t name_len = strlen(name);
    func->name = malloc(name_len);
    func->name[name_len] = '\0';
    strncpy(func->name, name, name_len);

    return func;
}

AST_Declaration* create_ident_decl(char* ident_name, data_t type, AST_Expression* expr)
{
    AST_Declaration* decl = (AST_Declaration*) malloc(sizeof(AST_Declaration));

    decl->expr = expr;
    decl->type_info.type = type;
    decl->type_info.params = NULL;
    decl->type_info.return_type = TYPE_VOID;

    size_t ident_len = strlen(ident_name);
    decl->name = malloc(ident_len);
    decl->name[ident_len] = '\0';
    strncpy(decl->name, ident_name, ident_len);

    return decl;
}

AST_Parameter* create_param_node(char* name, data_t type)
{
    AST_Parameter* param = (AST_Parameter*) malloc(sizeof(AST_Parameter));
    param->next = NULL;
    param->type = type;

    size_t name_len = strlen(name);
    param->name = malloc(name_len);
    param->name[name_len] = '\0';
    strncpy(param->name, name, name_len);

    return param;
}
