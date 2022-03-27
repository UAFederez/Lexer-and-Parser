#ifndef LANG_AST_EXPRESSION_H
#define LANG_AST_EXPRESSION_H

#include "Parser.h"
#include <cstddef>
#include "AST_Node.h"

struct Operator_Info
{
    TokenType op;
    int       precedence;
    bool      is_left_assoc;
    Expr_t    expr_type;
};

namespace ast {
    std::unique_ptr<Expression> parse_atom(ParserState*);
    std::unique_ptr<Expression> parse_expression(ParserState*, int min_prec = 0);
    std::unique_ptr<Expression> maybe_parse_func_call(ParserState*);
};

bool check_if_binary_op(ParserState*, Operator_Info*);
#endif
