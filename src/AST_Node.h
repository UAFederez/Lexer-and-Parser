#ifndef LANG_AST_NODE_H
#define LANG_AST_NODE_H

#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>

struct GraphvizDocument
{
    int next_id() { return curr_node_id++; }
    int curr_node_id;
    std::ostringstream oss;
    std::string file_name;
};

enum class Expr_t { 
    NONE           , ADD      , MUL        , DIV         , SUB           ,
    COMP_LT        , COMP_NEQ , COMP_LEQ   , ASSIGN      , COMP_GT       ,
    COMP_GEQ       , COMP_EQU , IDENTIFIER , INT_LITERAL , FLOAT_LITERAL ,
    STRING_LITERAL , NEGATE   , ARG        , CALL        ,
};

enum class Stmt_t { NONE, IF, FOR, DECL, RETURN, EXPR };
enum class Type_t { INT, FLOAT, FUNCTION, VOID };

namespace ast {
    class AST_Node
    {
        public:
            AST_Node() = default;
            virtual int output_graphviz(GraphvizDocument&) const = 0;
            virtual ~AST_Node() = default;
    };
    class Expression : public AST_Node
    {
        private:
            Expr_t expr_type = Expr_t::NONE;

            int64_t     int_value = 0;
            double      flt_value = 0.0;
            std::string str_value = {};

        public:
            Expression() = default;
            Expression(Expr_t expr_type, Expression* lhs = nullptr, Expression* rhs = nullptr):
                expr_type(expr_type), lhs(lhs), rhs(rhs) { }

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

            int output_graphviz(GraphvizDocument& doc) const override
            {
                static const char* EXPR_FMT   = "    expr_%d[label=\"{%s|{<f1>lhs|<f2>rhs}}\"];\n";
                static const char* EXPR_FMT_I = "    expr_%d[label=\"{%ld|{<f1>lhs|<f2>rhs}}\"];\n";
                static const char* EXPR_FMT_F = "    expr_%d[label=\"{%.2f|{<f1>lhs|<f2>rhs}}\"];\n";
                static const std::unordered_map<Expr_t, std::string> op_lexemes = {
                    { Expr_t::ADD   , "+" }, { Expr_t::SUB, "-" }, { Expr_t::MUL, "*" }, { Expr_t::DIV, "/" },
                    { Expr_t::ASSIGN, "=" }
                };

                char buffer[1024] = {};

                int expr_id = doc.next_id();
                switch(expr_type)
                {
                    case Expr_t::IDENTIFIER:
                        std::snprintf(buffer, 1024, EXPR_FMT, expr_id, str_value.c_str()); break;
                    case Expr_t::CALL:
                        std::snprintf(buffer, 1024, EXPR_FMT, expr_id, "\\<call\\>"); break;
                    case Expr_t::ARG:
                        std::snprintf(buffer, 1024, EXPR_FMT, expr_id, "\\<args\\>"); break;
                    case Expr_t::INT_LITERAL:
                        std::snprintf(buffer, 1024, EXPR_FMT_I, expr_id, int_value); 
                        break;
                    case Expr_t::FLOAT_LITERAL:
                        std::snprintf(buffer, 1024, EXPR_FMT_F, expr_id, flt_value); 
                        break;
                    default: 
                        if (op_lexemes.find(expr_type) != op_lexemes.end())
                            std::snprintf(buffer, 1024, EXPR_FMT, expr_id, op_lexemes.at(expr_type).c_str()); 
                        else
                            std::snprintf(buffer, 1024, EXPR_FMT, expr_id, "op"); 
                        break;
                }
                doc.oss << buffer;
                
                if(lhs)
                {
                    int lhs_id = lhs->output_graphviz(doc);
                    doc.oss << "    expr_" << expr_id << ":<f1> -> expr_" << lhs_id << ";\n";
                }
                if(rhs)
                {
                    int rhs_id = rhs->output_graphviz(doc);
                    doc.oss << "    expr_" << expr_id << ":<f2> -> expr_" << rhs_id << ";\n";
                }
                return expr_id;
            }
            ~Expression() override { }

            std::unique_ptr<Expression> lhs = nullptr;
            std::unique_ptr<Expression> rhs = nullptr;
    };

    //class VariableDecl : public Declaration
    //{

    //};
};

#endif
