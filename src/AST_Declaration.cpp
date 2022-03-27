#include "AST_Declaration.h"
#include "AST_Statement.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace ast 
{
    std::unique_ptr<Declaration> parse_declaration(ParserState* parser)
    {
        auto decl = maybe_parse_function_decl(parser);
        if(decl) 
            return decl;
        
        decl = maybe_parse_variable_decl(parser);
        if(decl) 
            return decl;

        parser->emit_error("Invalid declaration");
        return nullptr;
    }
    std::unique_ptr<Declaration> maybe_parse_function_decl(ParserState* parser)
    {
        Token* ident_token = parser->curr_token;
        parser->get_next_token();
        
        if (!parser->match_token(TOKEN_LEFT_PAREN))
        {
            parser->curr_token = ident_token;
            return nullptr;
        }

        // Definitely a function declaration at this point
        parser->get_next_token(); // consume '('

        std::unique_ptr<ParameterNode>  params     = nullptr;
        std::unique_ptr<ParameterNode>* curr_param = &params;
        bool expect_params = false;

        while(!parser->match_token(TOKEN_RIGHT_PAREN))
        {
            auto ident_type = ast::parse_ident_type_pair(parser);
            if(!ident_type && expect_params)
            {
                parser->curr_token = ident_token;
                return nullptr;
            }
            *curr_param = std::make_unique<ParameterNode>(ident_type->first, ident_type->second);
             curr_param = &(*curr_param)->next;

            if(parser->match_token(TOKEN_COMMA))
            {
                expect_params = true;
                parser->get_next_token();
            }
        }
        if(parser->match_token(TOKEN_RIGHT_PAREN))
        {
            parser->get_next_token(); // consume ')'
        }

        // Parse return type
        Type_t return_type = Type_t::VOID;
        if(parser->match_token(TOKEN_ARROW))
        {
            parser->get_next_token();
            if(!parser->match_token(TOKEN_IDENTIFIER))
            {
                parser->emit_error("Invalid identifier for datatype");
                parser->curr_token = ident_token;
                return nullptr;
            }

            Token* type_token = parser->curr_token;
            parser->get_next_token();

            if(type_token->lexeme != "int" && type_token->lexeme != "float") // @Cleanup
            {
                parser->emit_error("Custom datatypes are currently not supported");
                parser->curr_token = ident_token;
                return nullptr;
            }
            return_type = (type_token->lexeme == "int" ? Type_t::INT : Type_t::FLOAT);
        }

        // Parse block
        auto block = ast::parse_block(parser, true);
        if(block == nullptr)
        {
            parser->emit_error("Function must also be defined i.e. have a body upon declaration");
            parser->curr_token = ident_token;
            return nullptr;
        }
        return std::make_unique<FunctionDecl>(ident_token->lexeme, params.release(), 
                                              return_type, block.release());

    }
    std::unique_ptr<Declaration> maybe_parse_variable_decl(ParserState*) // TODO
    {
        return nullptr;
    }
    std::optional<std::pair<Type_t, std::string>> parse_ident_type_pair(ParserState* parser)
    {
        if(parser->match_token(TOKEN_IDENTIFIER))
        {
            Token* ident_token = parser->curr_token;
            parser->get_next_token();

            if(!parser->match_token(TOKEN_COLON))
            {
                parser->curr_token = ident_token;
                return std::nullopt;
            }
            parser->get_next_token();

            if(!parser->match_token(TOKEN_IDENTIFIER))
            {
                parser->curr_token = ident_token;
                return std::nullopt;
            }
            Token* type_token = parser->curr_token;
            parser->get_next_token();

            // @Cleanup put into separate function call
            if(type_token->lexeme != "int" && type_token->lexeme != "float")
            {
                parser->curr_token = ident_token;
                return std::nullopt;
            }
            Type_t type = (type_token->lexeme == "int" ? Type_t::INT : Type_t::FLOAT);
            return std::optional<std::pair<Type_t, std::string>> { {type, ident_token->lexeme } };
        }
        return std::nullopt;
    }
}