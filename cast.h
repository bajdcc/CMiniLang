//
// Project: CMiniLang
// Created by bajdcc
//

#ifndef CMINILANG_CAST_H
#define CMINILANG_CAST_H

#include "memory.h"

#define AST_NODE_MEM (32 * 1024)
#define AST_STR_MEM (16 * 1024)

namespace clib {

    enum ast_t {
        ast_root,
        ast_enum,
        ast_enum_unit,
        ast_var_global,
        ast_var_param,
        ast_var_local,
        ast_func,
        ast_param,
        ast_block,
        ast_exp,
        ast_exp_param,
        ast_stmt,
        ast_return,
        ast_sinop,
        ast_binop,
        ast_triop,
        ast_if,
        ast_while,
        ast_invoke,
        ast_empty,
        ast_id,
        ast_type,
        ast_cast,
        ast_string,
        ast_char,
        ast_uchar,
        ast_short,
        ast_ushort,
        ast_int,
        ast_uint,
        ast_long,
        ast_ulong,
        ast_float,
        ast_double,
    };

    enum ast_to_t {
        to_parent,
        to_prev,
        to_next,
        to_child,
    };

    // 结点
    struct ast_node {
        // 类型
        uint32_t flag;

        union {
#define DEFINE_NODE_DATA(t) LEX_T(t) _##t;
            DEFINE_NODE_DATA(char)
            DEFINE_NODE_DATA(uchar)
            DEFINE_NODE_DATA(short)
            DEFINE_NODE_DATA(ushort)
            DEFINE_NODE_DATA(int)
            DEFINE_NODE_DATA(uint)
            DEFINE_NODE_DATA(long)
            DEFINE_NODE_DATA(ulong)
            DEFINE_NODE_DATA(float)
            DEFINE_NODE_DATA(double)
#undef DEFINE_NODE_DATA
            const char *_string;
            struct {
                lexer_t type;
                sint ptr;
            } _type;
            struct {
                operator_t op;
                sint data;
            } _op;
            struct {
                sint _1, _2;
            } _ins;
        } data; // 数据

        // 树型数据结构，广义表
        ast_node *parent; // 父亲
        ast_node *prev; // 左兄弟
        ast_node *next; // 右兄弟
        ast_node *child; // 最左儿子
    };

    class cast {
    public:
        cast();
        ~cast() = default;

        ast_node *get_root();

        ast_node *new_node(ast_t type);
        ast_node *new_child(ast_t type, bool step = true);
        ast_node *new_sibling(ast_t type, bool step = true);

        ast_node *add_child(ast_node*);
        static ast_node *set_child(ast_node*, ast_node*);
        static ast_node *set_sibling(ast_node*, ast_node*);
        static int children_size(ast_node*);

        void set_id(ast_node *node, const string_t &str);
        void set_str(ast_node *node, const string_t &str);
        static std::string display_str(ast_node *node);

        void to(ast_to_t type);

        static void print(ast_node *node, int level, std::ostream &os);

        void reset();
    private:
        void init();

    private:
        memory_pool<AST_NODE_MEM> nodes; // 全局AST结点内存管理
        memory_pool<AST_STR_MEM> strings; // 全局字符串管理
        std::unordered_map<string_t, const char *> vars; // 变量名查找
        ast_node *root; // 根结点
        ast_node *current; // 当前结点
    };
}

#endif //CMINILANG_CAST_H
