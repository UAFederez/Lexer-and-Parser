#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Lexer.h"
#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Declaration.h"
#include "AST_Statement.h"
#include "GraphvizOutput.h"

char* load_program_source(const char* path, size_t* source_len)
{
    FILE* input_file = fopen(path, "rb");

    if(!input_file)
        return NULL;

    fseek(input_file, 0, SEEK_END);
    size_t input_len  = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char* input_string = (char*) malloc(input_len + 1);
    fread(input_string, input_len, 1, input_file);
    input_string[input_len] = '\0';
    
    *source_len = input_len;
    return input_string;
}

void print_expr(AST_Expression* expr, size_t indent)
{
    if (expr)
    {
        for (size_t i = 0; i < indent; i++)
            printf("  ");
        switch (expr->type)
        {
            case EXPR_ADD    : printf("+\n")                    ; break ;
            case EXPR_SUB    : printf("-\n")                    ; break ;
            case EXPR_MUL    : printf("*\n")                    ; break ;
            case EXPR_DIV    : printf("/\n")                    ; break ;
            case EXPR_ASSIGN : printf("=\n")                    ; break ;
            case EXPR_IDENT  : printf("%s\n", expr->ident_name) ; break ;
            case EXPR_INT    : printf("%ld\n", expr->int_value) ; break ;
            case EXPR_FLOAT  : printf("%f\n", expr->flt_value)  ; break ;
            case EXPR_CALL   : printf("call\n")                 ; break ;
            case EXPR_ARG    : printf("arg\n")                  ; break ;
            case EXPR_NEG    : printf("neg\n")                  ; break ;
            default: break;
        }
        print_expr(expr->lhs, indent + 1);
        print_expr(expr->rhs, indent + 1);
    }
}

void print_tokens(Token* token_stream)
{
    for( Token* tok = token_stream ; tok != NULL ; tok = tok->next )
    {
        printf("(line: %-2lld) ", tok->line_number);
        switch(tok->type)
        {
            case TOKEN_IDENTIFIER    : printf("[Ident ] %s\n",  tok->lexeme)     ; break ;
            case TOKEN_INT_LITERAL   : printf("[Int   ] %ld\n", tok->int_value)  ; break ;
            case TOKEN_FLOAT_LITERAL : printf("[Float ] %f\n",  tok->flt_value)  ; break ;
            case TOKEN_LEFT_PAREN    : printf("[LParen] %s\n",  tok->lexeme)     ; break ;
            case TOKEN_RIGHT_PAREN   : printf("[RParen] %s\n",  tok->lexeme)     ; break ;
            case TOKEN_EOF           : printf("[EOF   ] %s\n",  tok->lexeme)     ; break ;

            case TOKEN_COMP_LESS: case TOKEN_COMP_GREATER:
                printf("[Comp  ] %s\n", tok->lexeme);
            break;
            case KEYWORD_IF : case KEYWORD_FUNC : case KEYWORD_RETURN : case KEYWORD_ELSE :
                printf("[Keywd ] %s\n", tok->lexeme);
            break;

            default:
                printf("[ChTok ] %s\n", tok->lexeme);
            break;
        }
    }
}

int main()
{
    size_t source_len   = 0;
    char* source_string = load_program_source("sample_program.lang", &source_len);
    if(!source_string)
    {
        printf("[Error] Could not read input file!\n");
        return -1;
    }

    if(source_len == 0) 
    {
        printf("File is empty. No need to do anything...");
        return 0;
    }

    LexerState lexer_state;
    lexer_state.input_string = source_string;
    lexer_state.tokens       = NULL;
    lexer_state.input_len    = source_len;

    size_t lex_result = lexer_state.tokenize_string();

    printf("Lexer finished with status: %lld\n", lex_result);

    ParserState parser;
    parser.status       = PARSE_SUCCESS;
    parser.token_stream = lexer_state.tokens;
    parser.curr_token   = lexer_state.tokens;

    AST_Declaration *decl = parse_declaration(&parser);

    if(parser.status != PARSE_SUCCESS)
    {
        size_t errant_line = parser.curr_token->line_number;
        size_t pos_in_line = parser.curr_token->pos_in_line;
        switch(parser.status)
        {
            case PARSE_ERR_MALFORMED_EXPR: 
                printf("[Error] Malformed expression at line %lld:%lld\n", errant_line, pos_in_line);
                break;
            case PARSE_ERR_UNMATCHED_PAREN: 
                printf("[Error] Unmatched parentheses at line %lld\n", errant_line);
                break;
            case PARSE_ERR_INVALID_DECL:
                printf("[Error] Invalid declaration at line %lld\n", errant_line);
                break;
            case PARSE_ERR_INVALID_TYPE:
                printf("[Error] Unrecognized type name at line %lld. This language currently does not support user-defined types\n", errant_line);
                break;
            case PARSE_ERR_INVALID_PARAM:
                printf("[Error] Invalid parameter at line %lld.\n", errant_line);
                break;
            default: break;
        }
    } else {
        printf("Parser successful!\n");

        FILE* output_file = fopen("ast_output.gv", "w");
        GraphvizState gv;
        gv.curr_node_id = 0;
        gv.root    = decl;
        gv.out     = output_file;
        printf("Saving to dot file \'ast_output.gv\'...");
        print_graph(&gv);
        printf("Done!\n");
    }
    free(source_string);
    return 0;
}