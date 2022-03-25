#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "Lexer.h"


void LexerState::insert_token(const std::string& lexeme, enum TokenType type)
{
    tokens = do_insert_token(lexeme, type, tokens, curr_line_number, curr_pos_in_line, line_start_idx);
}

Token* LexerState::do_insert_token(const std::string& lexeme, enum TokenType type, Token* token_stream, size_t line_no, size_t line_pos, size_t line_idx)
{
    if(token_stream == NULL)
    {
        Token* new_token       = new Token();
        new_token->next        = NULL;
        new_token->prev        = NULL;
        new_token->type        = type;
        new_token->line_number = line_no;
        new_token->pos_in_line = line_pos;

        switch(type)
        {
            case TOKEN_IDENTIFIER:
                new_token->lexeme = lexeme;
            break;
            case TOKEN_INT_LITERAL:
                new_token->int_value = std::stol(lexeme);
            break;
            case TOKEN_FLOAT_LITERAL:
                new_token->flt_value = std::stod(lexeme);
            break;
            default:
                new_token->lexeme = lexeme;
            break;
        }
        return new_token;
    }
    token_stream->next       = do_insert_token(lexeme, type, token_stream->next, line_no, line_pos, line_idx);
    token_stream->next->prev = token_stream;
    return token_stream;
}

size_t LexerState::tokenize_string()
{
    curr_ch_idx        = 0;
    num_tokens         = 0;

    line_start_idx     = 0;
    curr_line_number   = 1;
    curr_pos_in_line   = 1;

    preprocess_string(&input_string[0], input_len);

    size_t status = LEX_SUCCESS;
    while(curr_ch_idx < input_len)
    {
        bool read_valid_token = true;
        while(isspace(input_string[curr_ch_idx]))
        {
            curr_pos_in_line++;
            curr_ch_idx++;
            if (input_string[curr_ch_idx] == '\n')
            {
                curr_line_number++;
                curr_pos_in_line = 1;
                line_start_idx = curr_ch_idx + 1;
            }
        }
        curr_char = input_string[curr_ch_idx];

        if(!curr_char) break;
        if(isalpha(curr_char) && (read_valid_token &= maybe_parse_identifier()))  continue;
        if(isdigit(curr_char) && (read_valid_token &= maybe_parse_num_literal())) continue;
        
        read_valid_token &= maybe_parse_operators();
        if (!read_valid_token)
        {
            printf("[Error] Unrecognized token at line %lu \"%c\"\n", curr_line_number, curr_char);
            printf("    \"");
            while(input_string[line_start_idx++] != '\n')
                printf("%c", input_string[line_start_idx - 1]);
            printf("\"\n");
            printf("\n\n");
            status = LEX_ERR_UNKNOWN_TOKEN;
            break;
        }
        curr_ch_idx++;
    }
    insert_token("EOF", TOKEN_EOF);
    return status;
}

size_t LexerState::maybe_parse_num_literal()
{
    size_t end_idx  = curr_ch_idx + 1;
    int num_periods = 0;

    char char_at_end = input_string[end_idx];
    while(end_idx < input_len && (isdigit(char_at_end) || char_at_end == '.'))
    {
        if(input_string[end_idx] == '.')
            num_periods++;
        char_at_end = input_string[end_idx++];
    }

    if(num_periods > 1)
        return LEX_ERR_INVALID_REAL;

    size_t num_len      = end_idx - curr_ch_idx;
    enum TokenType type = (num_periods == 0 ? TOKEN_INT_LITERAL : TOKEN_FLOAT_LITERAL);

    lexeme_buffer = input_string.substr(curr_ch_idx, num_len);

    insert_token(lexeme_buffer, type);
    curr_ch_idx = end_idx;

    return 1;
}


size_t LexerState::maybe_parse_identifier()
{
    static const std::array<std::string, 4> KEYWORDS = {
        "if", "else", "func", "return"
    };

    size_t end_idx = curr_ch_idx + 1;
    while(end_idx < input_len && (isalnum(input_string[end_idx]) || input_string[end_idx] == '_'))
        end_idx++;
    
    const size_t ident_len = end_idx - curr_ch_idx;
    lexeme_buffer = input_string.substr(curr_ch_idx, ident_len);

    enum TokenType type = TOKEN_IDENTIFIER;
    auto match = std::find(KEYWORDS.begin(), KEYWORDS.end(), lexeme_buffer);

    if (match != KEYWORDS.end())
        type = static_cast<TokenType>(KEYWORD_IF + (match - KEYWORDS.begin()));

    insert_token(lexeme_buffer, type);
    curr_ch_idx += ident_len;

    num_tokens++;
    return 1;
}


size_t LexerState::maybe_parse_operators()
{
    // Ordered in the same way as the corresponding TokenTypes
    static const char  SINGLE_CH_TOKENS[] = { 
        '+', '-', '*', '/', '=', '(', ')' , ',', ':', ';', '{', '}', '<', '>'
    };
    static const char* MULTI_CH_TOKENS[]  = { 
        "++", "--", "**", "==", "<=", ">=", "!=", "->"
    };

    bool found_operator = false;
    if (curr_ch_idx < input_len - 2)
    {
        char next_char = input_string[curr_ch_idx + 1];
        char token[3]  = { curr_char, next_char, 0 };

        for (int i = 0; i < 8; ++i)
        {
            if (strcmp(token, MULTI_CH_TOKENS[i]) == 0)
            {
                insert_token(token, static_cast<TokenType>(TOKEN_INCREMENT + i));
                found_operator = true;
                curr_ch_idx += 2;
                curr_pos_in_line += 2;
                curr_char = input_string[curr_ch_idx];
                break;
            }
        }
    }

    for (int i = 0; i < 14 && !found_operator; i++)
    {
        if (SINGLE_CH_TOKENS[i] == curr_char) 
        {
            char token[2]  = { curr_char, '\0' };
            insert_token(token, static_cast<TokenType>(TOKEN_OP_PLUS + i));
            found_operator = true;
            break;
        }
    }
    return found_operator;
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
