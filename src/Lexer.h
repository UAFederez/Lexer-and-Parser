#ifndef LANG_LEXER_H
#define LANG_LEXER_H

#include <stdbool.h>

enum LexerError {
    LEX_SUCCESS , LEX_ERR_UNKNOWN_TOKEN , LEX_ERR_INVALID_REAL , LEX_ERR_INVALID_INT ,
};

enum TokenType { 
    TOKEN_IDENTIFIER , TOKEN_INT_LITERAL  , TOKEN_FLOAT_LITERAL , TOKEN_STR_LITERAL  ,

    TOKEN_OP_PLUS    , TOKEN_OP_MINUS     , TOKEN_OP_MUL        , TOKEN_OP_DIV       ,
    TOKEN_OP_EQU     , TOKEN_LEFT_PAREN   , TOKEN_RIGHT_PAREN   , TOKEN_COMMA        ,
    TOKEN_COLON      , TOKEN_SEMICOLON    , TOKEN_LEFT_CBRACK   , TOKEN_RIGHT_CBRACK ,
    TOKEN_COMP_LESS  , TOKEN_COMP_GREATER ,

    TOKEN_INCREMENT  , TOKEN_DECREMENT    , TOKEN_POWER         , TOKEN_COMP_EQUAL   ,
    TOKEN_COMP_LEQ   , TOKEN_COMP_GEQ     , TOKEN_COMP_NEQ      , TOKEN_ARROW        ,

    KEYWORD_IF       , KEYWORD_ELSE       , KEYWORD_FUNC        , KEYWORD_RETURN
};

typedef struct TOKEN {
    enum   TokenType type;
    char*  lexeme;
    long   int_value;
    double flt_value;

    size_t line_number;
    size_t pos_in_line;
    size_t line_start_idx;

    struct TOKEN* next;
    struct TOKEN* prev;
} Token;

struct OperatorInfo {
    char* op;
    int   precedence;
    bool  is_left_assoc;
};

typedef struct {
    char*  input_string;
    size_t input_len;
    size_t num_tokens;
    Token* tokens;

    size_t curr_line_number;
    size_t curr_pos_in_line;
    size_t line_start_idx;

    char   curr_char;
    size_t curr_ch_idx;
    size_t max_lexeme_len;
    char*  lexeme_str;
} LexerState;

void read_next_char   (LexerState*);
void deallocate_tokens(LexerState*);

char * check_if_realloc       ( char* str, size_t capacity, size_t size);
void   insert_token           ( char*, enum TokenType, LexerState* );
Token* do_insert_token        ( char*, enum TokenType, Token*, size_t, size_t, size_t );
void   preprocess_string      ( char*, size_t);
size_t tokenize_string        ( LexerState* );
size_t maybe_parse_identifier ( LexerState* );
size_t maybe_parse_num_literal( LexerState* );
size_t maybe_parse_operators  ( LexerState* );
#endif
