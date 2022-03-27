#include "AST_Expression.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace ast {
    // Backtracks the current token to the identifier in the event 
    // that this is not a function call
    std::unique_ptr<Expression> maybe_parse_func_call(ParserState* parser)
    {
        Token* ident_token = parser->curr_token;
        auto ast_ident = std::make_unique<Expression>(Expr_t::IDENTIFIER, ident_token->lexeme);
        parser->get_next_token();

        if(parser->match_token(TOKEN_LEFT_PAREN))
        {
            parser->get_next_token(); // consume '('

            std::unique_ptr<Expression>  arg_root = nullptr;
            std::unique_ptr<Expression>* curr_arg = &arg_root;
            bool expecting_another_argument = false;

            while(!parser->match_token(TOKEN_RIGHT_PAREN))
            {
                auto arg = ast::parse_expression(parser);
                if(!arg && expecting_another_argument)
                {
                    parser->emit_error("Invalid argument!\n");
                    parser->curr_token = ident_token;
                    return nullptr;
                }
                *curr_arg = std::make_unique<Expression>(Expr_t::ARG, arg.release());
                 curr_arg = &(*curr_arg)->rhs;

                 if(parser->match_token(TOKEN_COMMA))
                 {
                     parser->get_next_token();
                     expecting_another_argument = true;
                 }
            }
            if(!parser->match_token(TOKEN_RIGHT_PAREN))
            {
                parser->emit_error("Unmatched parenthesis on function call");
                parser->curr_token = ident_token;
                return nullptr;
            }
            parser->get_next_token(); // consume ')'
            return std::make_unique<Expression>(Expr_t::CALL, ast_ident.release(), arg_root.release());
        } 
        parser->curr_token = ident_token;
        return nullptr;
    }
    std::unique_ptr<Expression> parse_atom(ParserState* parser)
    {
        std::unique_ptr<Expression> atom = nullptr;

        if(parser->match_token(TOKEN_IDENTIFIER))
        {
            atom = ast::maybe_parse_func_call(parser);
            if(atom == nullptr)
            {
                atom = std::make_unique<Expression>(Expr_t::IDENTIFIER, parser->curr_token->lexeme);
                parser->get_next_token();
            }
        }
        else if(parser->match_token(TOKEN_INT_LITERAL))
        {
            atom = std::make_unique<Expression>((int64_t) parser->curr_token->int_value);
            parser->get_next_token();
        }
        else if(parser->match_token(TOKEN_FLOAT_LITERAL))
        {
            atom = std::make_unique<Expression>(parser->curr_token->flt_value);
            parser->get_next_token();
        }
        else if(parser->match_token(TOKEN_LEFT_PAREN))
        {
            parser->get_next_token();   // consume '('

            atom = ast::parse_expression(parser);
            if (!parser->match_token(TOKEN_RIGHT_PAREN))
            {
                parser->emit_error("Unmatched parenthesis!\n");
                return nullptr;
            }
            parser->get_next_token(); // consume ')'
        }
        else if(parser->match_token(TOKEN_OP_MINUS))
        {
            parser->get_next_token(); // consume '-'
            atom = ast::parse_expression(parser); // TODO: actually do the negation
            atom = std::make_unique<Expression>(Expr_t::NEGATE, atom.release());
        }
        return atom;
    }
    std::unique_ptr<Expression> parse_expression(ParserState* parser, int min_prec)
    {
        auto result = ast::parse_atom(parser);

        if (!result)
        {
            parser->emit_error("Invalid expression!\n");
            return result;
        }

        bool is_binary_op = false;
        Operator_Info curr_op;

        while ((is_binary_op = check_if_binary_op(parser, &curr_op)) && curr_op.precedence >= min_prec)
        {
            Token* bin_op = parser->curr_token;
            parser->get_next_token();

            int next_min_prec = curr_op.precedence + (curr_op.is_left_assoc ? 1 : 0);
            auto rhs = ast::parse_expression(parser, next_min_prec);

            if (!rhs)
            {
                if (parser->status == PARSE_SUCCESS) parser->status = PARSE_ERR_MALFORMED_EXPR;
                parser->curr_token = bin_op;
                return nullptr;
            }

            // TODO: get the type of the expression
            result = std::make_unique<Expression>(curr_op.expr_type, result.release(), rhs.release());
        }
        return result;
    }
}

bool check_if_binary_op(ParserState* parser, Operator_Info* op_info)
{
    static const Operator_Info info[] = { 
        { TOKEN_OP_PLUS   , 1 , true , Expr_t::ADD      } , { TOKEN_OP_MINUS     , 1 , true, Expr_t::SUB      } ,
        { TOKEN_OP_DIV    , 2 , true , Expr_t::DIV      } , { TOKEN_OP_MUL       , 2 , true, Expr_t::MUL      } ,
        { TOKEN_COMP_LESS , 3 , true , Expr_t::COMP_LT  } , { TOKEN_COMP_GREATER , 3 , true, Expr_t::COMP_GT  } ,
        { TOKEN_COMP_LEQ  , 3 , true , Expr_t::COMP_LEQ } , { TOKEN_COMP_GEQ     , 3 , true, Expr_t::COMP_GEQ } ,
        { TOKEN_COMP_NEQ  , 4 , true , Expr_t::COMP_NEQ } , { TOKEN_COMP_EQUAL   , 4 , true, Expr_t::COMP_EQU } ,
        { TOKEN_OP_EQU    , 0 , false, Expr_t::ASSIGN   } ,
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