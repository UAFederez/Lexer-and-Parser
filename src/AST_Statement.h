#ifndef LANG_AST_STATEMENT_H
#define LANG_AST_STATEMENT_H

#include <memory>
#include <stdio.h>
#include <cstddef>

#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Declaration.h"
#include "AST_Node.h"

namespace ast 
{
    std::unique_ptr<Statement> parse_block(ParserState*, bool);
    std::unique_ptr<Statement> parse_statement(ParserState*);
    std::unique_ptr<Statement> parse_if_statement(ParserState*);
    std::unique_ptr<Statement> parse_return_statement(ParserState*);
}

#endif
