#ifndef LANG_AST_DECLARATION_H
#define LANG_AST_DECLARATION_H

#include <memory>
#include <cstddef>
#include <optional>

#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Node.h"

/**
    Declaration -> IDENTIFIER : TYPEIDENTIFIER { = Expression }; ?
                -> func IDENTIFIER ( ParamList ) -> TYPEIDENTIFIER ['{' Statement* '}']?
**/

// Statements can be declarations
// but declarations may also contain statements (function call)

namespace ast {
    std::unique_ptr<Declaration> parse_declaration(ParserState*);
    std::unique_ptr<Declaration> maybe_parse_function_decl(ParserState*);
    std::unique_ptr<Declaration> maybe_parse_variable_decl(ParserState*);
    std::optional<std::pair<Type_t, std::string>> parse_ident_type_pair(ParserState*);
};
#endif
