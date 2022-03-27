#include "AST_Statement.h"
#include <stdlib.h>

namespace ast 
{
    std::unique_ptr<Statement> parse_statement(ParserState* parser)
    {
        std::unique_ptr<Statement> stmt = nullptr;

        if (parser->match_token(KEYWORD_IF))
            stmt = ast::parse_if_statement(parser);
        else if (parser->match_token(KEYWORD_RETURN))
            stmt = ast::parse_return_statement(parser);
        else
        {
            auto expr = ast::parse_expression(parser);
            
            if (!parser->match_token(TOKEN_SEMICOLON))
            {
                parser->emit_error("Expected a semicolon");
                return nullptr;
            }
            parser->get_next_token();

            stmt = std::make_unique<ExprStatement>(false, expr.release());
        }
        return stmt;
    }

    std::unique_ptr<Statement> parse_block(ParserState* parser, bool require_braces)
    {
        bool begins_with_left_cbrack = parser->match_token(TOKEN_LEFT_CBRACK);

        if (require_braces && !begins_with_left_cbrack)
            return nullptr;

        if (begins_with_left_cbrack)
        {
            parser->get_next_token();

            std::unique_ptr<Statement>  stmts     = NULL;
            std::unique_ptr<Statement>* curr_stmt = &stmts;

            while (!parser->match_token(TOKEN_RIGHT_CBRACK))
            {
                std::unique_ptr<Statement> stmt = ast::parse_statement(parser);
                if (stmt == nullptr)
                {
                    parser->emit_error("No statement!\n");
                    return nullptr;
                }
                *curr_stmt = std::move(stmt);
                 curr_stmt = &(*curr_stmt)->next;
            }
            if (!match_token(parser, TOKEN_RIGHT_CBRACK))
            {
                parser->emit_error("No matching right bracket!\n");
                return nullptr;
            }
            parser->get_next_token();
            return stmts;
        }
        return ast::parse_statement(parser);
    }
    std::unique_ptr<Statement> parse_if_statement(ParserState* parser)
    {
        parser->get_next_token(); // consume 'if'
        std::unique_ptr<Expression> condition = ast::parse_expression(parser);
        std::unique_ptr<Statement > block     = ast::parse_block(parser, false);
        std::unique_ptr<Statement > else_blk  = nullptr;

        if(parser->match_token(KEYWORD_ELSE))
        {
            parser->get_next_token();
            else_blk = ast::parse_block(parser, false);
        }
        return std::make_unique<IfStatement>(condition.release(), block.release(), else_blk.release());
    }
    std::unique_ptr<Statement> parse_return_statement(ParserState* parser)
    {
        parser->get_next_token();
        std::unique_ptr<Expression> ret_expr = ast::parse_expression(parser);

        if (!parser->match_token(TOKEN_SEMICOLON))
        {
            parser->emit_error("Expected a semicolon");
            return nullptr;
        }
        parser->get_next_token();
        return std::make_unique<ExprStatement>(true, ret_expr.release());
    }
}