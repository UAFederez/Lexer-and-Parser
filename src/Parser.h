#pragma once
#ifndef LANG_PARSER_H
#define LANG_PARSER_H

#include "Lexer.h"
#include <cstddef>
#include <vector>

enum lex_error_t {
    ERR_NONE                  , ERR_LEX_UNRECOGNIZED_TOKEN , ERR_PARSE_MALFORMED_EXPR ,
    ERR_PARSE_UNMATCHED_PAREN , ERR_PARSE_INVALID_TYPE     , ERR_PARSE_INVALID_DECL   ,
    ERR_PARSE_INVALID_PARAM   , ERR_PARSE_IF_STMT_NO_BODY  ,
};

struct ErrorMessage
{
    std::string msg;
    std::size_t line_number;
    std::size_t pos_in_line;
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

    size_t curr_line_idx;
    size_t curr_pos_in_line;
    parse_status_t status; 

    void emit_error(const std::string& message);
    bool match_token(enum TokenType);
    bool get_next_token();

    std::vector<ErrorMessage> errors;
};

bool match_token     (ParserState*, enum TokenType);
bool get_next_token  (ParserState*);

#endif
