#include <string.h>
#include <ctype.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <memory>

#include "Lexer.h"
#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Declaration.h"
#include "AST_Statement.h"
#include "GraphvizOutput.h"
#include "AST_Node.h"

std::string load_program_source(const char* path)
{
    std::ifstream input_file(path);

    if (!input_file)
    {
        std::fprintf(stderr, "[Error] could not load file: %s\n", path);
        return {};
    }
    std::ostringstream oss;
    oss << input_file.rdbuf();

    return oss.str();
}

void print_tokens(Token* token_stream)
{
    for( Token* tok = token_stream ; tok != NULL ; tok = tok->next )
    {
        printf("(line: %-2llu) ", tok->line_number);
        switch(tok->type)
        {
            case TOKEN_IDENTIFIER    : printf("[Ident ] %s\n",  tok->lexeme.c_str())     ; break ;
            case TOKEN_INT_LITERAL   : printf("[Int   ] %ld\n", tok->int_value)          ; break ;
            case TOKEN_FLOAT_LITERAL : printf("[Float ] %f\n",  tok->flt_value)          ; break ;
            case TOKEN_LEFT_PAREN    : printf("[LParen] %s\n",  tok->lexeme.c_str())     ; break ;
            case TOKEN_RIGHT_PAREN   : printf("[RParen] %s\n",  tok->lexeme.c_str())     ; break ;
            case TOKEN_EOF           : printf("[EOF   ] %s\n",  tok->lexeme.c_str())     ; break ;

            case TOKEN_COMP_LESS: case TOKEN_COMP_GREATER:
                printf("[Comp  ] %s\n", tok->lexeme.c_str());
            break;
            case KEYWORD_IF : case KEYWORD_FUNC : case KEYWORD_RETURN : case KEYWORD_ELSE :
                printf("[Keywd ] %s\n", tok->lexeme.c_str());
            break;

            default:
                printf("[ChTok ] %s\n", tok->lexeme.c_str());
            break;
        }
    }
}

int main()
{
    std::string source_string = load_program_source("sample_program.lang");
    if(!source_string.size())
    {
        printf("[Error] Could not read input file!\n");
        return -1;
    }

    if(source_string.size() == 0) 
    {
        printf("File is empty. No need to do anything...");
        return 0;
    }

    LexerState lexer_state;
    lexer_state.input_len    = source_string.size();
    lexer_state.input_string = std::move(source_string);
    lexer_state.tokens       = NULL;

    size_t lex_result = lexer_state.tokenize_string();

    // TODO: debug this status result code
    // printf("Lexer finished with status: %llu\n", lex_result);
    //print_tokens(lexer_state.tokens);

    ParserState parser;
    parser.status       = PARSE_SUCCESS;
    parser.token_stream = lexer_state.tokens;
    parser.curr_token   = lexer_state.tokens;
    
    auto stmt = ast::parse_declaration(&parser);
    printf("done parsing!\n");
    if(!parser.errors.empty()) 
    {
        printf("Number of errors: %lld\n", parser.errors.size());
        for(const ErrorMessage& e : parser.errors) 
        {
            // TODO: These line numbers and line positions are incorrect
            // for now, improve on keeping track of this and potentially
            // implement in the future a way to print out the actual line
            // with a marker that represents where the error is based on the
            // position of the start of the errant token 
            std::printf("[Error] on line %lld at position %lld:\n\t%s\n", 
                        e.line_number, e.pos_in_line, e.msg.c_str());
        }
    } else
    {
        GraphvizDocument doc;
        doc.curr_node_id = 0;
        doc.file_name    = "test.gv";

        doc.oss << "digraph G {\n    node[shape=record fontname=Arial];\n";
        stmt->output_graphviz(doc);
        doc.oss << "}\n";

        std::ofstream output_file("ast_output.gv");
        output_file << doc.oss.str() << '\n';
        std::cout << doc.oss.str() << '\n';
    }
    return 0;
}
