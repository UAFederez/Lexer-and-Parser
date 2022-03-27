#include "Parser.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

bool match_token(ParserState* parser, enum TokenType type)
{
    return parser->curr_token != NULL && parser->curr_token->type == type;
}

void ParserState::emit_error(const std::string& message)
{
    errors.push_back({ message, curr_line_idx, curr_pos_in_line });
}

bool ParserState::match_token(enum TokenType type)
{
    return curr_token-> type != TOKEN_EOF && curr_token->type == type;
}

bool ParserState::get_next_token()
{
    if(curr_token->type != TOKEN_EOF) 
    {
        curr_line_idx    = curr_token->line_number;
        curr_pos_in_line = curr_token->pos_in_line;
        curr_token       = curr_token->next;
        return true;
    }
    return false;
}

bool get_next_token (ParserState* parser)
{
    if(parser->curr_token->type != TOKEN_EOF) 
    {
        parser->curr_line_idx    = parser->curr_token->line_number;
        parser->curr_pos_in_line = parser->curr_token->pos_in_line;
        parser->curr_token       = parser->curr_token->next;
        return true;
    }
    return false;
}
