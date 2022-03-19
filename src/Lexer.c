#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "Lexer.h"

size_t tokenize_string(LexerState* state)
{
    state->curr_ch_idx    = 0;
    state->max_lexeme_len = 0;
    state->lexeme_str     = 0;
    state->num_tokens     = 0;

    state->line_start_idx   = 0;
    state->curr_line_number = 1;
    state->curr_pos_in_line = 1;

    preprocess_string(state->input_string, state->input_len);

    while(state->curr_ch_idx < state->input_len)
    {
        bool read_valid_token = true;
        while(isspace(state->input_string[state->curr_ch_idx]))
        {
            state->curr_pos_in_line++;
            state->curr_ch_idx++;
            if (state->input_string[state->curr_ch_idx] == '\n')
            {
                state->curr_line_number++;
                state->curr_pos_in_line = 1;
                state->line_start_idx = state->curr_ch_idx + 1;
            }
        }
        state->curr_char = state->input_string[state->curr_ch_idx];

        if(!state->curr_char) break;
        if(isalpha(state->curr_char) && (read_valid_token &= maybe_parse_identifier(state)))  continue;
        if(isdigit(state->curr_char) && (read_valid_token &= maybe_parse_num_literal(state))) continue;
        
        read_valid_token &= maybe_parse_operators(state);
        if (!read_valid_token)
        {
            printf("[Error] Unrecognized token at line %lld \"%c\"\n", state->curr_line_number, state->curr_char);
            printf("    \"");
            while(state->input_string[state->line_start_idx++] != '\n')
                printf("%c", state->input_string[state->line_start_idx - 1]);
            printf("\"\n");
            printf("\n\n");
            return LEX_ERR_UNKNOWN_TOKEN;
        }
        state->curr_ch_idx++;
    }
    return LEX_SUCCESS;
}

size_t maybe_parse_num_literal(LexerState* state)
{
    size_t end_idx  = state->curr_ch_idx + 1;
    int num_periods = 0;

    while(end_idx < state->input_len && 
            (isdigit(state->input_string[end_idx]) || state->input_string[end_idx] == '.'))
    {
        if(state->input_string[end_idx] == '.')
            num_periods++;
        end_idx++;
    }

    if(num_periods > 1)
        return LEX_ERR_INVALID_REAL;

    size_t num_len      = end_idx - state->curr_ch_idx;
    enum TokenType type = (num_periods == 0 ? TOKEN_INT_LITERAL : TOKEN_FLOAT_LITERAL);

    state->lexeme_str = check_if_realloc(state->lexeme_str, state->max_lexeme_len, num_len);
    state->lexeme_str[num_len] = '\0';
    strncpy(state->lexeme_str, state->input_string + state->curr_ch_idx, num_len);

    insert_token(state->lexeme_str, type, state);
    state->curr_ch_idx = end_idx;

    return 1;
}

size_t maybe_parse_identifier (LexerState* state )
{
    static const char* KEYWORDS[] = {
        "if", "else", "func", "return"
    };

    size_t end_idx = state->curr_ch_idx + 1;
    while(end_idx < state->input_len && 
          (isalnum(state->input_string[end_idx]) || state->input_string[end_idx] == '_'))
    {
        end_idx++;
    }
    
    size_t ident_len  = end_idx - state->curr_ch_idx;
    state->lexeme_str = check_if_realloc(state->lexeme_str, state->max_lexeme_len, ident_len);
    state->lexeme_str[ident_len] = 0;
    strncpy(state->lexeme_str, (state->input_string + state->curr_ch_idx), ident_len);
    state->lexeme_str[ident_len] = 0;

    enum TokenType type = TOKEN_IDENTIFIER;
    for (int i = 0; i < 4; ++i) 
    {
        if(strcmp(state->lexeme_str, KEYWORDS[i]) == 0)
        {
            type = KEYWORD_IF + i;
            break;
        }
    }

    insert_token(state->lexeme_str, type, state);
    state->curr_ch_idx += ident_len;

    // Reset the lexeme string buffer to be reused
    memset(state->lexeme_str, 0, ident_len);
    if(ident_len < state->max_lexeme_len)
        state->max_lexeme_len = ident_len;

    state->num_tokens++;
    return 1;
}

void insert_token(char* lexeme, enum TokenType type, LexerState* lexer)
{
    lexer->tokens = do_insert_token(lexeme, type, lexer->tokens, 
                                    lexer->curr_line_number, 
                                    lexer->curr_pos_in_line,
                                    lexer->line_start_idx);
}

Token* do_insert_token(char* lexeme, enum TokenType type, Token* token_stream, size_t line_no, size_t line_pos, size_t line_idx)
{
    if(token_stream == NULL)
    {
        size_t lex_length      = strlen(lexeme);
        Token* new_token       = (Token*) malloc(sizeof(Token));
        new_token->next        = NULL;
        new_token->prev        = NULL;
        new_token->type        = type;
        new_token->line_number = line_no;
        new_token->pos_in_line = line_pos;

        switch(type)
        {
            case TOKEN_IDENTIFIER:
            {
                new_token->lexeme = (char *)malloc(lex_length + 1);
                new_token->lexeme[lex_length] = 0;
                strncpy(new_token->lexeme, lexeme, lex_length);
            }
            break;
            case TOKEN_INT_LITERAL:
                new_token->int_value = strtol(lexeme, NULL, 10);
            break;
            case TOKEN_FLOAT_LITERAL:
                new_token->flt_value = strtod(lexeme, NULL);
            break;
            default:
            {
                size_t len = strlen(lexeme);
                new_token->lexeme = (char*) malloc(len);
                new_token->lexeme[len] = 0;
                strncpy(new_token->lexeme, lexeme, len);
            }
            break;
        }
        return new_token;
    }
    token_stream->next       = do_insert_token(lexeme, type, token_stream->next, line_no, line_pos, line_idx);
    token_stream->next->prev = token_stream;
    return token_stream;
}

size_t maybe_parse_operators(LexerState* state)
{
    // Ordered in the same way as the corresponding TokenTypes
    static const char  SINGLE_CH_TOKENS[] = { 
        '+', '-', '*', '/', '=', '(', ')' , ',', ':', ';', '{', '}', '<', '>'
    };
    static const char* MULTI_CH_TOKENS[]  = { 
        "++", "--", "**", "==", "<=", ">=", "!=", "->"
    };

    bool found_operator = false;
    if (state->curr_ch_idx < state->input_len - 2)
    {
        char next_char = state->input_string[state->curr_ch_idx + 1];
        char token[3]  = { state->curr_char, next_char, 0 };

        for (int i = 0; i < 8; ++i)
        {
            if (strcmp(token, MULTI_CH_TOKENS[i]) == 0)
            {
                insert_token(token, TOKEN_INCREMENT + i, state);
                found_operator = true;
                state->curr_ch_idx += 2;
                state->curr_pos_in_line += 2;
                state->curr_char = state->input_string[state->curr_ch_idx];
                break;
            }
        }
    }

    for (int i = 0; i < 14 && !found_operator; i++)
    {
        if (SINGLE_CH_TOKENS[i] == state->curr_char) 
        {
            char token[2]  = { state->curr_char, '\0' };
            insert_token(token, TOKEN_OP_PLUS + i, state);
            found_operator = true;
            break;
        }
    }
    return found_operator;
}

void deallocate_tokens(LexerState* state)
{
    Token* curr_token = state->tokens;
    while (curr_token != NULL)
    {
        Token* to_delete = curr_token;
        curr_token = curr_token->next;

        if (to_delete->type != TOKEN_INT_LITERAL && to_delete->type != TOKEN_FLOAT_LITERAL)
        {
            if(to_delete->lexeme) 
                free(to_delete->lexeme);
        }

        free(to_delete);
    }
}
void preprocess_string(char* input_string, size_t input_len)
{
    if(input_len > 2) 
    {
        // i++ handles skipping the newline when comment is replaced with blanks
        for(size_t i = 0; i < input_len; i++)
        {
            if(input_string[i] == '\r') input_string[i] = ' ';

            if (input_string[i] == '/' && input_string[i + 1] == '/')
            {
                while(input_string[i] != '\n') 
                    input_string[i++] = ' ';
            }
        }
    }
}

char* check_if_realloc(char* str, size_t capacity, size_t size)
{
    if(size >= capacity)
        return (char*) realloc(str, capacity + size);
    return str;
}
