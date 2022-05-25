#pragma once

#include <memory>
#include "AST_Declaration.h"

namespace ast {
    class Interpreter
    {
        public:
            Interpreter() = default;
            Interpreter(std::unique_ptr<ast::Declaration>& root):
                root(std::move(root))
            {
            }

            void run() const;
        private:
            std::unique_ptr<ast::Declaration> root;
    };
}
