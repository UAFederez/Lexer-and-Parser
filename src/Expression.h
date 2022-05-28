#pragma once
#ifndef LANG_AST_EXPRESSION_H
#define LANG_AST_EXPRESSION_H

#include "Parser.h"
#include <cstddef>
#include <unordered_map>
#include "Node.h"

enum class Expr_t { 
    NONE           , ADD      , MUL        , DIV         , SUB           ,
    COMP_LT        , COMP_NEQ , COMP_LEQ   , ASSIGN      , COMP_GT       ,
    COMP_GEQ       , COMP_EQU , IDENTIFIER , INT_LITERAL , FLOAT_LITERAL ,
    STRING_LITERAL , NEGATE   , ARG        , CALL        ,
};
class Type_t { 
public:
    enum Value
    {
        INT, FLOAT, FUNCTION, VOID, POINTER 
    };

    Type_t(const Value& v):
        value(v) { }

    std::string to_string() const {
        static const std::unordered_map<Value, std::string> str {
            { INT , "Integer"  }, { FLOAT  , "Float"   }, { FUNCTION, "Function" },
            { VOID, "Void"     }, { POINTER, "Pointer" },
        };
        return str.at(value);
    }
    bool operator==(const Value& v) const { return value == v; }
private:
    Value value;
};

struct Operator_Info
{
    TokenType op;
    int       precedence;
    bool      is_left_assoc;
    Expr_t    expr_type;
};

namespace ast {
    class Expression : public AST_Node
    {
    private:
        Expr_t expr_type = Expr_t::NONE;

        int64_t     int_value = 0;
        double      flt_value = 0.0;
        std::string str_value = {};

        std::unique_ptr<Expression> lhs_ = nullptr;
        std::unique_ptr<Expression> rhs_ = nullptr;
    public:
        Expression() = default;
        Expression(Expr_t expr_type, Expression* lhs = nullptr, Expression* rhs = nullptr):
            expr_type(expr_type), lhs_(lhs), rhs_(rhs) { }

        Expression(int64_t int_value ) : 
            expr_type(Expr_t::INT_LITERAL), int_value(int_value) {}

        Expression(double flt_value ) : 
            expr_type(Expr_t::FLOAT_LITERAL), flt_value(flt_value) {}

        Expression(Expr_t type, const std::string& str) : 
            expr_type(type), str_value(str) { }

        int64_t get_int()  const    { return int_value; }
        double  get_flt()  const    { return flt_value; }
        Expr_t  get_type() const    { return expr_type; }
        std::string get_str() const { return str_value; }

        int output_graphviz(GraphvizDocument& doc) const override;
        ~Expression() override { }
        
        std::unique_ptr<ast::Expression>* rhs() { return &rhs_; }
        std::unique_ptr<ast::Expression>* lhs() { return &lhs_; }
    };


    std::unique_ptr<Expression> parse_atom(ParserState*);
    std::unique_ptr<Expression> parse_expression(ParserState*, int min_prec = 0);
    std::unique_ptr<Expression> maybe_parse_func_call(ParserState*);
};

bool check_if_binary_op(ParserState*, Operator_Info*);
#endif
