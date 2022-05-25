#pragma once
#ifndef LANG_LEXER_H
#define LANG_LEXER_H

#include <stdbool.h>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <array>
#include <algorithm>

enum LexerError 
{
    LEX_SUCCESS , LEX_ERR_UNKNOWN_TOKEN , LEX_ERR_INVALID_REAL , LEX_ERR_INVALID_INT ,
};

enum TokenType 
{ 
    TOKEN_IDENTIFIER , TOKEN_INT_LITERAL  , TOKEN_FLOAT_LITERAL , TOKEN_STR_LITERAL  ,

    TOKEN_OP_PLUS    , TOKEN_OP_MINUS     , TOKEN_OP_MUL        , TOKEN_OP_DIV       ,
    TOKEN_OP_EQU     , TOKEN_LEFT_PAREN   , TOKEN_RIGHT_PAREN   , TOKEN_COMMA        ,
    TOKEN_COLON      , TOKEN_SEMICOLON    , TOKEN_LEFT_CBRACK   , TOKEN_RIGHT_CBRACK ,
    TOKEN_COMP_LESS  , TOKEN_COMP_GREATER ,

    TOKEN_INCREMENT  , TOKEN_DECREMENT    , TOKEN_POWER         , TOKEN_COMP_EQUAL   ,
    TOKEN_COMP_LEQ   , TOKEN_COMP_GEQ     , TOKEN_COMP_NEQ      , TOKEN_ARROW        ,

    KEYWORD_IF       , KEYWORD_ELSE       , KEYWORD_FUNC        , KEYWORD_RETURN     ,
    TOKEN_EOF        ,
};

struct Token 
{
    enum   TokenType type;
    long   int_value;
    double flt_value;
    std::string lexeme = {};

    size_t line_number;
    size_t pos_in_line;
    size_t line_start_idx;

    Token* next;
    Token* prev;
};

struct LexerState 
{
    std::string input_string;
    size_t input_len;
    size_t num_tokens;
    Token* tokens;

    size_t curr_line_number;
    size_t curr_pos_in_line;
    size_t line_start_idx;

    char   curr_char;
    size_t curr_ch_idx;

    std::string lexeme_buffer;

    size_t tokenize_string();
    bool maybe_parse_identifier();
    bool maybe_parse_num_literal();
    bool maybe_parse_str_literal();
    bool maybe_parse_operators();

    ~LexerState() 
    {
        Token* curr_token = tokens;
        while (curr_token != NULL) 
        {
            Token* to_delete = curr_token;
            curr_token = curr_token->next;
            delete to_delete;
        }
    }

    void insert_token(const std::string&, enum TokenType);
    Token* do_insert_token(const std::string&, enum TokenType, Token*);
};
void preprocess_string(std::string&, size_t);
#endif
