#include "Declaration.h"
#include "Statement.h"

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

    std::unique_ptr<Statement> parse_var_decl_statement(ParserState* parser)
    {
        Token* first_token = parser->curr_token;

        if (!parser->match_token(TOKEN_IDENTIFIER))
        {
            parser->curr_token = first_token;
            return nullptr;
        }
        auto opt_decl = parse_ident_type_pair(parser);
        if (!opt_decl)
        {
            parser->curr_token = first_token;
            return nullptr;
        }
        std::unique_ptr<Expression> expr = nullptr;
        if (parser->match_token(TOKEN_OP_EQU))
        {
            parser->get_next_token();
            expr = parse_expression(parser);
        }
        auto decl = std::make_unique<VariableDecl>(opt_decl->first,  opt_decl->second, expr.release());
        return std::make_unique<VarDeclStatement>(decl.release());
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
             curr_param = (*curr_param)->get_next();

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
    int ParameterNode::output_graphviz(GraphvizDocument& doc) const 
    {
        static const char* fmt       = "    param_%d[label=\"{ParameterNode|{<f1>name|<f2>type|<f3>next}}\"];\n";
        static const char* fmt_name  = "    str_%d[label=\"{\\\"%s\\\"}\"];\n";
        static const char* fmt_value = "    str_%d[label=\"{%s}\"];\n";

        char buffer[1024] = {};

        int param_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt, param_id);
        doc.oss << buffer;

        int name_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt_name, name_id, name.c_str());
        doc.oss << buffer;

        int type_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt_value, type_id, type.to_string().c_str());
        doc.oss << buffer;

        doc.oss << "    param_" << param_id << ":<f1> -> str_" << name_id << ";\n";
        doc.oss << "    param_" << param_id << ":<f2> -> str_" << type_id << ";\n";
        if(next)
        {
            int next_id = next->output_graphviz(doc);
            doc.oss << "    param_" << param_id << ":<f3> -> param_" << next_id << ";\n";
        }
        return param_id; 
    }


    int VariableDecl::output_graphviz(GraphvizDocument& doc) const
    {
        static const char* fmt       = "    decl_%d[label=\"{%s|{<f1>type|<f2>expr}}\"];\n";
        static const char* fmt_value = "    str_%d[label=\"{%s}\"];\n";
        char buffer[512];

        int decl_id = doc.next_id();
        std::snprintf(buffer, 512, fmt, decl_id, name.c_str());
        doc.oss << buffer;

        int type_id = doc.next_id();
        std::snprintf(buffer, 512, fmt_value, type_id, get_type().to_string());
        doc.oss << buffer;

        doc.oss << "    decl_" << decl_id << ":<f1> -> str_" << type_id << ";\n";

        if (expr)
        {
            int expr_id = expr->output_graphviz(doc);
            doc.oss << "    decl_" << decl_id << ":<f2> -> expr_" << expr_id << ";\n";
        }
        return decl_id;
    }

    int VarDeclStatement::output_graphviz(GraphvizDocument& doc) const
    {
        static const char* fmt = "    stmt_%d[label=\"{VarDeclStatement|{<f1>decl|<f2>next}}\"]\n";

        int stmt_id = doc.next_id();
        int decl_id = decl->output_graphviz(doc);

        char buffer[512];
        std::snprintf(buffer, 512, fmt, stmt_id);
        doc.oss << buffer;

        doc.oss << "    stmt_" << stmt_id << ":<f1> -> decl_" << decl_id << ";\n";
        if(next)
        {
            int next_id = next->output_graphviz(doc);
            doc.oss << "    stmt_" << stmt_id << ":<f2> -> stmt_" << next_id << ";\n";
        }
        return stmt_id;
    }


    int FunctionDecl::output_graphviz(GraphvizDocument& doc) const
    {
        static const char* fmt        = "    decl_%d[label=\"{FunctionDecl | {<f1>name |<f2> params|<f3> ret_type|<f4> body|<f5>next}}\"];\n";
        static const char* fmt_name   = "    str_%d[label=\"{\\\"%s\\\"}\"];\n";
        static const char* fmt_value  = "    str_%d[label=\"{%s}\"];\n";

        char buffer[1024] = {};

        int decl_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt, decl_id);
        doc.oss << buffer;

        int name_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt_name, name_id, name.c_str());
        doc.oss << buffer;

        int ret_type_id = doc.next_id();
        std::snprintf(buffer, 1024, fmt_value, ret_type_id, return_type.to_string().c_str());
        doc.oss << buffer;

        doc.oss << "    decl_" << decl_id << ":<f1> -> str_" << name_id     << ";\n";
        doc.oss << "    decl_" << decl_id << ":<f3> -> str_" << ret_type_id << ";\n";

        if(params)
        {
            int param_id = params->output_graphviz(doc);
            doc.oss << "    decl_" << decl_id << ":<f2> -> param_" << param_id << ";\n";
        }

        if(body)
        {
            int body_id = body->output_graphviz(doc);
            doc.oss << "    decl_" << decl_id << ":<f4> -> stmt_" << body_id << ";\n";
        }

        if(next)
        {
            int next_id = next->output_graphviz(doc);
            doc.oss << "    decl_" << decl_id << ":<f5> -> decl_" << next_id << ";\n";
        }
        return decl_id;
    }
}
