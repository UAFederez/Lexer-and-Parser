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

namespace ast {

    class AST_Node
    {
        public:
            AST_Node() = default;
            virtual int output_graphviz(GraphvizDocument&) const = 0;
            virtual ~AST_Node() = default;
    };
};

#endif
