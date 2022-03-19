#ifndef LANG_AST_STATEMENT_H
#define LANG_AST_STATEMENT_H

#include <stdbool.h>
#include <stdio.h>

#include "Parser.h"
#include "AST_Expression.h"
#include "AST_Declaration.h"

typedef enum { 
    STMT_EXPR   , STMT_DECL , STMT_IF , STMT_FOR ,
    STMT_RETURN
} stmt_t;

typedef struct AST_STMT {
    stmt_t type;
    struct AST_STMT *next;
     
    // represents condition if type -> STMT_IF | STMT_FOR
    // return value if type == STMT_RETURN
    // or simply an Expression
    AST_Expression * expr;
    AST_Declaration* decl;

    struct AST_STMT *body;     // Body of if's, for's, etc..
    struct AST_STMT *else_blk;
} AST_Statement;

AST_Statement* parse_block       (ParserState* parser, bool);
AST_Statement* parse_statement   (ParserState*);
AST_Statement* parse_if_statement(ParserState*);
AST_Statement* create_statement  (stmt_t, AST_Expression*, AST_Declaration*, AST_Statement*, AST_Statement*);
AST_Statement* parse_return_statement(ParserState*);

#endif
