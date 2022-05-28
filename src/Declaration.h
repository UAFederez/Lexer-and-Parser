#pragma once
#ifndef LANG_AST_DECLARATION_H
#define LANG_AST_DECLARATION_H

#include <memory>
#include <cstddef>
#include <optional>

#include "Parser.h"
#include "Expression.h"
#include "Statement.h"
#include "Node.h"

/**
    Declaration -> IDENTIFIER : TYPEIDENTIFIER { = Expression }; ?
                -> func IDENTIFIER ( ParamList ) -> TYPEIDENTIFIER ['{' Statement* '}']?
**/

// Statements can be declarations
// but declarations may also contain statements (function call)

namespace ast 
{
    struct ParameterNode : public AST_Node
    {
        ParameterNode(Type_t type, const std::string& name):
            type(type), name(name) { }

        int output_graphviz(GraphvizDocument& doc) const override;
        ~ParameterNode() { }

        std::unique_ptr<ParameterNode>* get_next() { return &next; } 

        private:
            Type_t      type;
            std::string name;
            std::unique_ptr<ParameterNode> next = nullptr;
    };

    class Declaration : public AST_Node
    {
        public:
            Declaration() = default;
            Declaration(Type_t type):
                basic_type(type) { }

            Type_t get_type() const { return basic_type; }
            virtual int output_graphviz(GraphvizDocument& doc) const = 0;
            virtual ~Declaration() = default;

            void set_next(std::unique_ptr<ast::Declaration>& n)
            {
                next = std::move(n);
            }
        protected:
            Type_t basic_type;
            std::unique_ptr<Declaration > next = nullptr;
    };

    class FunctionDecl : public Declaration
    {
        public:
            FunctionDecl():
                Declaration(Type_t::FUNCTION) { }
            FunctionDecl(const std::string& name, ParameterNode* params, Type_t ret_type, ast::Statement* body):
                Declaration(Type_t::FUNCTION), name(name), params(params), return_type(ret_type), body(body) { }

            int output_graphviz(GraphvizDocument& doc) const override;
            
        private:
            std::string name = {};
            std::unique_ptr<ParameterNode> params = nullptr;
            Type_t return_type = Type_t::VOID;
            std::unique_ptr<Statement> body = nullptr;
    };

    class VariableDecl : public Declaration
    {
        public:
            VariableDecl():
                Declaration(Type_t::VOID) {}
            VariableDecl(Type_t type, const std::string& name, Expression* expr = nullptr):
                Declaration(type), name(name), expr(expr) {}
            int output_graphviz(GraphvizDocument& ) const override;
            ~VariableDecl() override { }
        private:
            std::string name = {};
            std::unique_ptr<Expression> expr = nullptr;
    };

    class VarDeclStatement : public Statement
    {
    public:
        VarDeclStatement() = default;
        VarDeclStatement(VariableDecl* decl):
            decl(decl) { }
        int output_graphviz(GraphvizDocument&) const override;
        ~VarDeclStatement() override { }
    private:
        std::unique_ptr<VariableDecl> decl = nullptr;
    };

    std::unique_ptr<Declaration> parse_declaration(ParserState*);
    std::unique_ptr<Declaration> maybe_parse_function_decl(ParserState*);
    std::unique_ptr<Declaration> maybe_parse_variable_decl(ParserState*);
};
#endif
