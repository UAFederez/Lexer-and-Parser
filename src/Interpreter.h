#pragma once

#include <memory>
#include <unordered_map>
#include "Declaration.h"

namespace ast {
    class SymbolTable
    {
        public:
            void enter_context();
            void exit_context();

            // Query the symbol table, starting from the current context and 
            // travels up the tables while the symbol has not been found
            ast::Expression* query(const std::string& sym_name);
        private:
            std::vector<std::unordered_map<std::string, ast::Expression*>> symbols;
    };

    class Interpreter
    {
        public:
            Interpreter() = default;
            Interpreter(std::unique_ptr<ast::Declaration>& root):
                root(std::move(root))
            {

            }

            void run() const;
            void execute_expression(ast::Expression* expr);
        private:
            std::unique_ptr<ast::Declaration> root;
            ast::SymbolTable symbols;
    };
}
