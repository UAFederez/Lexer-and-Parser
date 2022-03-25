#include "AST_Expression.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

// TODO: Check robustness against errant expressions
AST_Expression* parse_atom(ParserState* parser)
{
    AST_Expression* atom = NULL;

    if(parser->curr_token == NULL) return NULL;

    if(match_token(parser, TOKEN_IDENTIFIER))
    {
        atom = maybe_parse_call(parser);

        // If parsing the function call failed catastrophically
        if(parser->status != PARSE_SUCCESS) return NULL;

        if(atom == NULL)
        {
            atom = create_expr_ident(parser->curr_token->lexeme, NULL, NULL);
            get_next_token(parser);
        }
    }
    else if(match_token(parser, TOKEN_INT_LITERAL))
    {
        atom = create_expr_int(parser->curr_token->int_value);
        get_next_token(parser);
    }
    else if(match_token(parser, TOKEN_FLOAT_LITERAL))
    {
        atom = create_expr_float(parser->curr_token->flt_value);
        get_next_token(parser);
    }
    else if(match_token(parser, TOKEN_LEFT_PAREN))
    {
        Token* paren_tok = parser->curr_token;
        get_next_token(parser);
        atom = parse_expression(parser, 0);

        if(!match_token(parser, TOKEN_RIGHT_PAREN))
        {
            parser->status     = PARSE_ERR_UNMATCHED_PAREN;
            parser->curr_token = paren_tok;
            return NULL;
        }
        get_next_token(parser);
    }
    else if(match_token(parser, TOKEN_OP_MINUS))
    {
        Token* op_token = parser->curr_token;
        get_next_token(parser);
        
        AST_Expression* expr = parse_atom(parser);
        if (expr != NULL)
        {
            if (expr->type == EXPR_INT)
                return create_expr_int(-expr->int_value);
            else if (expr->type == EXPR_FLOAT)
                return create_expr_float(-expr->flt_value);
            else
                return create_expr(EXPR_NEG, expr, NULL);
        }

        parser->curr_token = op_token;
        return NULL;
    }
    return atom;
}

AST_Expression* parse_expression(ParserState* parser, size_t min_prec)
{
    AST_Expression* result = parse_atom(parser);
    if (result == NULL) 
    {
        if(parser->status == PARSE_SUCCESS) parser->status = PARSE_ERR_MALFORMED_EXPR;
        return result;
    }

    bool is_binary_op = false;
    Operator_Info curr_op;

    while ((is_binary_op = check_if_binary_op(parser, &curr_op)) && curr_op.precedence >= min_prec)
    {
        Token* bin_op = parser->curr_token;
        get_next_token(parser);

        size_t next_min_prec = 0;
        if(curr_op.is_left_assoc)
            next_min_prec = curr_op.precedence + 1;
        else
            next_min_prec = curr_op.precedence;

        expr_t type = EXPR_ADD;
        switch(curr_op.op)
        {
            case TOKEN_OP_PLUS   : type = EXPR_ADD       ; break ;
            case TOKEN_OP_MINUS  : type = EXPR_SUB       ; break ;
            case TOKEN_OP_MUL    : type = EXPR_MUL       ; break ;
            case TOKEN_OP_DIV    : type = EXPR_DIV       ; break ;
            case TOKEN_OP_EQU    : type = EXPR_ASSIGN    ; break ;
            case TOKEN_COMP_LESS : type = EXPR_COMP_LESS ; break ;
            default: break;
        }
        AST_Expression* rhs = parse_expression(parser, next_min_prec);
        if (rhs == NULL)
        {
            if (parser->status == PARSE_SUCCESS)
                parser->status = PARSE_ERR_MALFORMED_EXPR;

            parser->curr_token = bin_op;
            return NULL;
        }
        result = create_expr(type, result, rhs);
    }
    // TODO: error checking
    return result;
}

AST_Expression* maybe_parse_call(ParserState* parser)
{
    Token* id_token = parser->curr_token; // for backtracking
    get_next_token(parser);

    if (match_token(parser, TOKEN_LEFT_PAREN))
    {
        get_next_token(parser);

        AST_Expression*  arg_root = NULL;
        AST_Expression** curr_arg = &arg_root;

        while(!match_token(parser, TOKEN_RIGHT_PAREN))
        {
            AST_Expression* arg = parse_expression(parser, 0);
            if (arg == NULL)
            {
                parser->status     = PARSE_ERR_UNMATCHED_PAREN;
                parser->curr_token = id_token;
                return NULL;
            }
            if(*curr_arg == NULL)
                *curr_arg = create_expr(EXPR_ARG, arg, NULL);
            curr_arg = &((*curr_arg)->rhs);

            // TODO: handle commas on function arguments
            if(match_token(parser, TOKEN_COMMA))    
                get_next_token(parser);
        }
        get_next_token(parser); // consume ')'
        AST_Expression* func_id = create_expr_ident(id_token->lexeme, NULL, NULL);
        return create_expr(EXPR_CALL, func_id, arg_root);
    } 
    parser->curr_token = id_token;
    return NULL;
}

bool check_if_binary_op(ParserState* parser, Operator_Info* op_info)
{
    static const Operator_Info info[] = { 
        { TOKEN_OP_PLUS   , 1 , true } , { TOKEN_OP_MINUS     , 1 , true } ,
        { TOKEN_OP_DIV    , 2 , true } , { TOKEN_OP_MUL       , 2 , true } ,
        { TOKEN_COMP_LESS , 3 , true } , { TOKEN_COMP_GREATER , 3 , true } ,
        { TOKEN_COMP_LEQ  , 3 , true } , { TOKEN_COMP_GEQ     , 3 , true } ,
        { TOKEN_COMP_NEQ  , 4 , true } , { TOKEN_COMP_EQUAL   , 4 , true } ,
        { TOKEN_OP_EQU    , 0 , false} ,
    };
    
    if (parser->curr_token != NULL)
    {
        for (size_t i = 0; i < 11; ++i)
        {
            if (parser->curr_token->type == info[i].op)
            {
                *op_info = info[i];
                return true;
            }
        }
    }
    return false;
}

AST_Expression* create_expr(expr_t type, AST_Expression* lhs, AST_Expression* rhs)
{
    AST_Expression* expr = new AST_Expression();
    expr->type = type;
    expr->lhs  = lhs;
    expr->rhs  = rhs;
    return expr;
}

AST_Expression* create_expr_ident(const std::string& ident, AST_Expression* lhs, AST_Expression* rhs)
{
    AST_Expression* expr = create_expr(EXPR_IDENT, lhs, rhs);
    expr->ident_name = ident;
    return expr;
}

AST_Expression* create_expr_int(long int_val)
{
    AST_Expression* expr = create_expr(EXPR_INT, NULL, NULL);
    expr->int_value = int_val;
    return expr;
}

AST_Expression* create_expr_float(double flt_val)
{
    AST_Expression* expr = create_expr(EXPR_FLOAT, NULL, NULL);
    expr->flt_value = flt_val;
    return expr;
}

void free_expression(AST_Expression* expr)
{
    if(expr != NULL)
    {
        if(expr->lhs) free_expression(expr->lhs);
        if(expr->rhs) free_expression(expr->rhs);

        delete expr;
    }
}
