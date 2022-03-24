#ifndef LANG_PARSER_H
#define LANG_PARSER_H

#include "Lexer.h"
#include <stdbool.h>

/**
    Program       -> Declaration* | \e
    Declaration   -> IDENTIFIER : TYPEIDENTIFIER {= Expression}?
                     | func Identifier ({ IDENTIFIER : TYPEIDENTIFIER ,?}*) { -> TYPEIDENTIFIER }?
                       \{ {Statement}* \}
    
    Statement     -> Expression | Declaration_S | If_Statement | For_loop | Return
    
    Expression    -> Expression BinaryOp Expression | FunctionCall | IDENTIFIER | Literal
    FunctionCall  -> Identifier ( { IDENTIFIER : TYPEIDENTIFIER ,?}* )
    Literal       -> LITERAL_FLOAT | LITERAL_INT

    Declaration_S -> Identifier : TypeIdentifier {= Expression}? ;
    Return        -> return Expression ;
    If_Statement  -> if Expression \{? Statement+ \}? { else \{? Statement \}? }?
**/

enum error_t {
    ERR_NONE                  , ERR_LEX_UNRECOGNIZED_TOKEN , ERR_PARSE_MALFORMED_EXPR ,
    ERR_PARSE_UNMATCHED_PAREN , ERR_PARSE_INVALID_TYPE     , ERR_PARSE_INVALID_DECL   ,
    ERR_PARSE_INVALID_PARAM   , ERR_PARSE_IF_STMT_NO_BODY  ,
};

struct ErrorList {
    error_t type;
    size_t line_number;
    size_t pos_in_line;
    Token* errant_token;

    ErrorList* next;
};

struct Operator_Info
{
    TokenType op;
    size_t precedence;
    bool   is_left_assoc;
};

// Note: maybe the better approach is to record the current line number on each
// get_next_token() and have an emit_err() function, that way there's no need to 
// roll back the current token just to get the errant line number
// 
// But also, perhaps when the non-terminal parsing fails e.g parse_expression() == NULL
// then it internally rolls back the curr_token pointer from where it started, idk...
// Decide on this later on
enum parse_status_t 
{
    PARSE_SUCCESS          , PARSE_ERR_MALFORMED_EXPR , PARSE_ERR_UNMATCHED_PAREN ,
    PARSE_ERR_INVALID_TYPE , PARSE_ERR_INVALID_DECL   , PARSE_ERR_INVALID_PARAM   ,
};

struct ParserState
{
    Token* token_stream;
    Token* curr_token;

    parse_status_t  status; 
    ErrorList *errors;
    size_t curr_line_idx;
    size_t curr_pos_in_line;

    bool match_token(enum TokenType);
    bool get_next_token();
    void emit_error(error_t type);
};

bool match_token     (ParserState*, enum TokenType);
bool get_next_token  (ParserState*);

void emit_error(ParserState* parser, error_t type);
ErrorList* do_append_error(ErrorList*, error_t, Token *, ParserState*);

#endif
