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
            stmt = parse_var_decl_statement(parser);
            std::unique_ptr<Expression> expr = nullptr; 
            
            // If this wasn't a declaration maybe it is an expression
            if (!stmt)
            {
                expr = ast::parse_expression(parser);
                stmt = std::make_unique<ExprStatement>(false, expr.release());
            }

            if (!parser->match_token(TOKEN_SEMICOLON))
            {
                parser->emit_error("Expected a semicolon");
                return nullptr;
            }
            parser->get_next_token();
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

    int ExprStatement::output_graphviz(GraphvizDocument& doc) const
    {
        static const char* expr_fmt  = "    stmt_%d[label=\"{%s|{<f1>expr|<f2>next}}\"];\n";

        int stmt_id = doc.next_id();
        char buffer[1024] = {};

        std::snprintf(buffer, 1024, expr_fmt, stmt_id, is_return_stmt ? "ReturnStatement" : "ExprStatement");
        doc.oss << buffer << '\n'; 

        if(expr)
        {
            int expr_id = expr->output_graphviz(doc);
            doc.oss << "    stmt_" << stmt_id << ":<f1> -> expr_" << expr_id << ";\n";
        }
        if(next)
        {
            int next_id = next->output_graphviz(doc);
            doc.oss << "    stmt_" << stmt_id << ":<f2> -> stmt_" << next_id << ";\n";
        }
        return stmt_id;
    }

    int IfStatement::output_graphviz(GraphvizDocument& doc) const
    {
        static const char* IF_FMT   = "    stmt_%d[label=\"{IfStatement|{<f1>condition|<f2>then|<f3>else|<f4>next}}\"];\n";

        int if_id = doc.next_id();
        char buffer[1024] = {};

        std::snprintf(buffer, 1024, IF_FMT, if_id);
        doc.oss << buffer << '\n'; 

        int cond_id = condition->output_graphviz(doc);
        doc.oss << "    stmt_" << if_id << ":<f1> -> expr_" << cond_id << ";\n";

        if(body)
        {
            int then_id = body->output_graphviz(doc);
            doc.oss << "    stmt_" << if_id << ":<f2> -> stmt_" << then_id << ";\n";
        }
        if(else_blk)
        {
            int else_id = else_blk->output_graphviz(doc);
            doc.oss << "    stmt_" << if_id << ":<f3> -> stmt_" << else_id << ";\n";
        }
        if(next)
        {
            int next_id = next->output_graphviz(doc);
            doc.oss << "    stmt_" << if_id << ":<f4> -> stmt_" << next_id << ";\n";
        }
        return if_id;
    }

}