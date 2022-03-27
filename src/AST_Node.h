#ifndef LANG_AST_NODE_H
#define LANG_AST_NODE_H

#include <iostream>
#include <memory>
#include <sstream>

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

    class Statement : public AST_Node
    {
        public:
            Statement() = default;
            Statement(Stmt_t type): stmt_type(type) { }
            virtual int output_graphviz(GraphvizDocument& doc) const = 0;
            virtual ~Statement() = default;

            Stmt_t     get_type() const { return stmt_type; }
            Statement* get_next()       { return next.get(); }
            void append_next(Statement* new_stmt) { next = do_append_next(next, new_stmt); }
            std::unique_ptr<Statement> next = nullptr;
        protected:
            Stmt_t stmt_type = Stmt_t::NONE;
            std::unique_ptr<Statement> do_append_next(std::unique_ptr<Statement>& root, 
                                                      Statement* stmt) 
            {
                if(root == nullptr)
                    return std::unique_ptr<Statement>(stmt);

                root->next = do_append_next(root->next, stmt);
                return std::move(root);
            }
    };

    class IfStatement : public Statement
    {
        public:
            IfStatement():
                Statement(Stmt_t::IF) { }
            IfStatement(Expression* expr, Statement* body = nullptr, Statement* else_blk = nullptr):
                Statement(Stmt_t::IF), condition(expr), body(body), else_blk(else_blk) { }

            int output_graphviz(GraphvizDocument& doc) const override 
            {
                static const char* IF_FMT   = "    stmt_%d[label=\"{IfStatement|{<f1>condition|<f2>then|<f3>else|<f4>next}}\"];\n";

                int if_id = doc.next_id();
                char buffer[1024] = {};

                std::snprintf(buffer, 1024, IF_FMT, if_id);
                doc.oss << buffer << '\n'; 

                int cond_id = condition->output_graphviz(doc);
                doc.oss << "    stmt_" << if_id << ":<f1> -> expr_" << cond_id << ";\n";

                if(body)
                {
                    int then_id = body->output_graphviz(doc);
                    doc.oss << "    stmt_" << if_id << ":<f2> -> stmt_" << then_id << ";\n";
                }
                if(else_blk)
                {
                    int else_id = else_blk->output_graphviz(doc);
                    doc.oss << "    stmt_" << if_id << ":<f3> -> stmt_" << else_id << ";\n";
                }
                if(next)
                {
                    int next_id = next->output_graphviz(doc);
                    doc.oss << "    stmt_" << if_id << ":<f4> -> stmt_" << next_id << ";\n";
                }
                return if_id;
            }
            ~IfStatement() override { }
        private:
            std::unique_ptr<Expression> condition = nullptr;
            std::unique_ptr<Statement > body      = nullptr;
            std::unique_ptr<Statement > else_blk  = nullptr;
    };

    class ExprStatement : public Statement
    {
        public:
            ExprStatement(bool is_return):
                Statement(is_return ? Stmt_t::RETURN : Stmt_t::EXPR) { 
                is_return_stmt = is_return; 
            }

            ExprStatement(bool is_return, Expression* expr):
                Statement(is_return ? Stmt_t::RETURN : Stmt_t::EXPR), expr(expr) { 
                is_return_stmt = is_return; 
            }

            int output_graphviz(GraphvizDocument& doc) const override 
            {
                static const char* expr_fmt  = "    stmt_%d[label=\"{%s|{<f1>expr|<f2>next}}\"];\n";

                int stmt_id = doc.next_id();
                char buffer[1024] = {};

                std::snprintf(buffer, 1024, expr_fmt, stmt_id, is_return_stmt ? "ReturnStatement" : "ExprStatement");
                doc.oss << buffer << '\n'; 

                if(expr)
                {
                    int expr_id = expr->output_graphviz(doc);
                    doc.oss << "    stmt_" << stmt_id << ":<f1> -> expr_" << expr_id << ";\n";
                }
                if(next)
                {
                    int next_id = next->output_graphviz(doc);
                    doc.oss << "    stmt_" << stmt_id << ":<f2> -> stmt_" << next_id << ";\n";
                }
                return stmt_id;
            }
            ~ExprStatement() override { }
        private:
            bool is_return_stmt = false;
            std::unique_ptr<Expression> expr;
    };

    struct ParameterNode : public AST_Node
    {
        ParameterNode(Type_t type, const std::string& name):
            type(type), name(name) { }
        Type_t      type;
        std::string name;
        std::unique_ptr<ParameterNode> next = nullptr;

        int output_graphviz(GraphvizDocument& doc) const override 
        {
            const char* fmt      = "    param_%d[label=\"{ParameterNode|{<f1>name|<f2>type|<f3>next}}\"];\n";
            const char* fmt_name = "    str_%d[label=\"{\\\"%s\\\"}\"];\n";

            char buffer[1024] = {};

            int param_id = doc.next_id();
            std::snprintf(buffer, 1024, fmt, param_id);
            doc.oss << buffer;

            int name_id = doc.next_id();
            std::snprintf(buffer, 1024, fmt_name, name_id, name.c_str());
            doc.oss << buffer;

            doc.oss << "    param_" << param_id << ":<f1> -> str_" << name_id << ";\n";
            if(next)
            {
                int next_id = next->output_graphviz(doc);
                doc.oss << "    param_" << param_id << ":<f3> -> param_" << next_id << ";\n";
            }
            return param_id; 
        }

        ~ParameterNode() { }
    };

    class Declaration : public AST_Node
    {
        public:
            Declaration() = default;
            Declaration(Type_t type):
                basic_type(type) { }
            virtual int output_graphviz(GraphvizDocument& doc) const = 0;
            virtual ~Declaration() = default;

        protected:
            Type_t basic_type;
            std::unique_ptr<Declaration > next = nullptr;
    };

    class FunctionDecl : public Declaration
    {
        public:
            FunctionDecl():
                Declaration(Type_t::FUNCTION) { }
            FunctionDecl(const std::string& name, ParameterNode* params, Type_t ret_type, Statement* body):
                Declaration(Type_t::FUNCTION), name(name), params(params), return_type(ret_type), body(body) { }

            int output_graphviz(GraphvizDocument& doc) const override 
            {
                static const char* fmt      = "    decl_%d[label=\"{FunctionDecl | {<f1>name |<f2> params|<f3> return|<f4> body|<f5>next}}\"];\n";
                static const char* fmt_name = "    str_%d[label=\"{\\\"%s\\\"}\"];\n";

                char buffer[1024] = {};

                int decl_id = doc.next_id();
                std::snprintf(buffer, 1024, fmt, decl_id);
                doc.oss << buffer;

                int name_id = doc.next_id();
                std::snprintf(buffer, 1024, fmt_name, name_id, name.c_str());
                doc.oss << buffer;

                doc.oss << "    decl_" << decl_id << ":<f1> -> str_" << name_id << ";\n";

                if(params)
                {
                    int param_id = params->output_graphviz(doc);
                    doc.oss << "    decl_" << decl_id << ":<f2> -> param_" << param_id << ";\n";
                }

                if(body)
                {
                    int body_id = body->output_graphviz(doc);
                    doc.oss << "    decl_" << decl_id << ":<f4> -> stmt_" << body_id << ";\n";
                }
                return decl_id;
            }
        private:
            std::string name = {};
            std::unique_ptr<ParameterNode> params = nullptr;
            [[ maybe_unused ]] Type_t return_type = Type_t::VOID;
            std::unique_ptr<Statement> body = nullptr;
    };

    //class VariableDecl : public Declaration
    //{

    //};
};

#endif
