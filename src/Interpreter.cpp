#include "Interpreter.h"

namespace ast {
    void Interpreter::run() const 
    {
        
    }

    void Interpreter::execute_expression(ast::Expression* expr)
    {
        if(expr->get_type() == Expr_t::CALL)
        {
            // Execute the call instruction
        } else
        {
            // Otherwise, it may be a binary expression or a single value

        }
    }
}

