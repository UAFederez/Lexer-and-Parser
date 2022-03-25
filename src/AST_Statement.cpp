#include "AST_Statement.h"
#include <stdlib.h>

void free_declaration(AST_Declaration*);

AST_Statement* parse_statement(ParserState* parser)
{
    AST_Statement* stmt = NULL;
    if (match_token(parser, KEYWORD_IF))
        stmt = parse_if_statement(parser);
    else if (match_token(parser, KEYWORD_RETURN))
        stmt = parse_return_statement(parser);
    else
    {
        AST_Expression* expr = parse_expression(parser, 0);
        if(expr)
            stmt = create_statement(STMT_EXPR, expr, NULL, NULL, NULL);
        if (match_token(parser, TOKEN_SEMICOLON))
            get_next_token(parser);
    }
    return stmt;
}

AST_Statement* parse_return_statement(ParserState* parser)
{
    get_next_token(parser); // consume return;
    AST_Expression* expr = parse_expression(parser, 0);
    if (expr == NULL)
    {
        // TODO: emit_error (...)
    }
    if (match_token(parser, TOKEN_SEMICOLON))
        get_next_token(parser);
    return create_statement(STMT_RETURN, expr, NULL, NULL, NULL);
}

AST_Statement* parse_if_statement(ParserState* parser)
{
    get_next_token(parser);   // consume 'if'
    AST_Expression* condition = parse_expression(parser, 0);

    if (condition == NULL)
    {
        emit_error(parser, ERR_PARSE_IF_STMT_NO_BODY);
        return NULL;
    }
    AST_Statement *body     = parse_block(parser, false);
    AST_Statement *else_blk = NULL;

    if (match_token(parser, KEYWORD_ELSE))
    {
        get_next_token(parser);
        else_blk = parse_block(parser, false);
    }
    return create_statement(STMT_IF, condition, NULL, body, else_blk);
}

AST_Statement* parse_block(ParserState* parser, bool require_braces)
{
    bool begins_with_left_cbrack = match_token(parser, TOKEN_LEFT_CBRACK);

    if (require_braces && !begins_with_left_cbrack)
        return NULL;

    if (begins_with_left_cbrack)
    {
        get_next_token(parser);

        AST_Statement*  stmts     = NULL;
        AST_Statement** curr_stmt = &stmts;

        while (!match_token(parser, TOKEN_RIGHT_CBRACK))
        {
            AST_Statement* stmt = parse_statement(parser);
            if (stmt == NULL)
            {
                fprintf(stderr, "No statement!\n");
                return NULL;
            }
                
            if (*curr_stmt == NULL) *curr_stmt = stmt;
            curr_stmt = &((*curr_stmt)->next);
        }
        if (!match_token(parser, TOKEN_RIGHT_CBRACK))
        {
            printf("No matching right bracket!\n");
            return NULL; // @Leak
        }
        get_next_token(parser);
        return stmts;
    }
    return parse_statement(parser);
}

AST_Statement* create_statement(stmt_t type, 
                                AST_Expression  *expr, 
                                AST_Declaration *decl, 
                                AST_Statement   *body,
                                AST_Statement   *else_blk)
{
    AST_Statement* stmt = new AST_Statement();
    stmt->next     = NULL;
    stmt->type     = type;
    stmt->expr     = expr;
    stmt->decl     = decl;
    stmt->body     = body;
    stmt->else_blk = else_blk;
    return stmt;
}

void free_statement(AST_Statement* stmt)
{
    if(stmt != NULL)
    {
        if(stmt->expr)
            free_expression(stmt->expr);
        if(stmt->body)
            free_statement(stmt->body);
        if(stmt->else_blk)
            free_statement(stmt->else_blk);
        if(stmt->decl)
            free_declaration(stmt->decl);
        if(stmt->next)
            free_statement(stmt->next);
        delete stmt;
    }
}
