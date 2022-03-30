#ifndef LANG_AST_STATEMENT_H
#define LANG_AST_STATEMENT_H

#include <memory>
#include <stdio.h>
#include <cstddef>
#include <optional>
#include <utility>

#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Node.h"

enum class Stmt_t { NONE, IF, FOR, DECL, RETURN, EXPR };

namespace ast 
{
    class Statement : public AST_Node
    {
        public:
            Statement() = default;
            Statement(Stmt_t type): stmt_type(type) { }
            virtual int output_graphviz(GraphvizDocument& doc) const = 0;
            virtual ~Statement() = default;

            Stmt_t get_type() const { return stmt_type; }
            std::unique_ptr<Statement> next = nullptr;
        protected:
            Stmt_t stmt_type = Stmt_t::NONE;
    };

    class IfStatement : public Statement
    {
        public:
            IfStatement():
                Statement(Stmt_t::IF) { }
            IfStatement(Expression* expr, Statement* body = nullptr, Statement* else_blk = nullptr):
                Statement(Stmt_t::IF), condition(expr), body(body), else_blk(else_blk) { }

            int output_graphviz(GraphvizDocument& doc) const override;
            ~IfStatement() override { }
        private:
            std::unique_ptr<Expression> condition = nullptr;
            std::unique_ptr<Statement > body      = nullptr;
            std::unique_ptr<Statement > else_blk  = nullptr;
    };

    class ExprStatement : public Statement
    {
        public:
            ExprStatement(bool is_return):
                Statement(is_return ? Stmt_t::RETURN : Stmt_t::EXPR) { 
                is_return_stmt = is_return; 
            }

            ExprStatement(bool is_return, Expression* expr):
                Statement(is_return ? Stmt_t::RETURN : Stmt_t::EXPR), expr(expr) { 
                is_return_stmt = is_return; 
            }

            int output_graphviz(GraphvizDocument& doc) const override;
            ~ExprStatement() override { }
        private:
            bool is_return_stmt = false;
            std::unique_ptr<Expression> expr;
    };

    std::unique_ptr<Statement> parse_block(ParserState*, bool);
    std::unique_ptr<Statement> parse_statement(ParserState*);
    std::unique_ptr<Statement> parse_var_decl_statement(ParserState*);
    std::unique_ptr<Statement> parse_if_statement(ParserState*);
    std::unique_ptr<Statement> parse_return_statement(ParserState*);
    std::optional<std::pair<Type_t, std::string>> parse_ident_type_pair(ParserState*);
}

#endif
