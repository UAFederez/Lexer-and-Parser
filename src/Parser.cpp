#include "Parser.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

bool match_token(ParserState* parser, enum TokenType type)
{
    return parser->curr_token != NULL && parser->curr_token->type == type;
}

void emit_error(ParserState* parser, lex_error_t type)
{
    parser->errors = do_append_error(parser->errors, type, parser->curr_token, parser);
}

// TODO: 
// Maybe there's a better way to organize this. The only reason that the 
// curr_line_idx and curr_pos_in_line is retrieved from the parser instead 
// of the current token is because the current token may be null
ErrorList* do_append_error(ErrorList* root, lex_error_t type, Token* errant_token, ParserState* parser)
{
    if(root == NULL)
    {
        ErrorList* new_err = (ErrorList*) malloc(sizeof(ErrorList));

        new_err->type = type;
        new_err->next = NULL;
        new_err->errant_token = errant_token;
        new_err->line_number  = parser->curr_line_idx;
        new_err->pos_in_line  = parser->curr_pos_in_line;
        return new_err;
    }
    root->next = do_append_error(root->next, type, errant_token, parser);
    return root;
}

bool get_next_token (ParserState* parser)
{
    if(parser->curr_token->type != TOKEN_EOF) 
    {
        parser->curr_line_idx = parser->curr_token->line_number;
        parser->curr_token    = parser->curr_token->next;
        return true;
    }
    return false;
}
