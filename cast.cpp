//
// Project: CMiniLang
// Created by bajdcc
//

#include <cstring>
#include <iostream>
#include <iomanip>
#include "cast.h"

namespace clib {

    cast::cast() {
        init();
    }

    void cast::init() {
        root = new_node(ast_root);
        current = root;
    }

    ast_node *cast::get_root() const {
        return root;
    }

    ast_node *cast::new_node(ast_t type) {
        auto node = nodes.alloc<ast_node>();
        memset(node, 0, sizeof(ast_node));
        node->flag = type;
        return node;
    }

    ast_node *cast::set_child(ast_node *node, ast_node *child) {
        child->parent = node;
        if (node->child == nullptr) { // 没有孩子
            node->child = child;
            child->prev = child->next = child;
        } else { // 有孩子，添加到末尾
            child->prev = node->child->prev;
            child->next = node->child;
            node->child->prev->next = child;
            node->child->prev = child;
        }
        return node;
    }
    ast_node *cast::set_sibling(ast_node *node, ast_node *sibling) {
        sibling->parent = node->parent;
        sibling->prev = node;
        sibling->next = node->next;
        node->next = sibling;
        return sibling;
    }

    ast_node *cast::add_child(ast_node *node) {
        return set_child(current, node);
    }

    ast_node *cast::new_child(ast_t type, bool step) {
        auto node = new_node(type);
        set_child(current, node);
        if (step)
            current = node;
        return node;
    }

    ast_node *cast::new_sibling(ast_t type, bool step) {
        auto node = new_node(type);
        set_sibling(current, node);
        if (step)
            current = node;
        return node;
    }

    void cast::to(ast_to_t type) {
        switch (type) {
            case to_parent:
                current = current->parent;
                break;
            case to_prev:
                current = current->prev;
                break;
            case to_next:
                current = current->next;
                break;
            case to_child:
                current = current->child;
                break;
        }
    }

    void cast::set_id(ast_node *node, const string_t &str) {
        auto f = vars.find(str);
        if (f != vars.end()) {
            node->data._string = f->second;
        } else {
            set_str(node, str);
            vars.insert(std::make_pair(str, node->data._string));
        }
    }

    void cast::set_str(ast_node *node, const string_t &str) {
        auto len = str.length();
        auto s = strings.alloc_array<char>(len + 1);
        memcpy(s, str.c_str(), len);
        s[len] = 0;
        node->data._string = s;
    }

    void cast::reset() {
        nodes.clear();
        strings.clear();
        vars.clear();
        init();
    }

    template<class T>
    static void ast_recursion(ast_node *node, int level, std::ostream &os, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, level, os);
            return;
        }
        f(i, level, os);
        i = i->next;
        while (i != node) {
            f(i, level, os);
            i = i->next;
        }
    }

    void cast::print(ast_node *node, int level, std::ostream &os) const {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto l, auto &os) { this->print(n, l, os); };
        auto type = (ast_t) node->flag;
        switch (type) {
            case ast_root: // 根结点，全局声明
                ast_recursion(node->child, level, os, rec);
                break;
            case ast_enum: // 枚举
                os << "enum {" << std::endl;
                ast_recursion(node->child, level + 1, os, rec);
                os << "};" << std::endl;
                break;
            case ast_enum_unit: // 枚举（等式）
                os << std::setw(level << 2) << "";
                rec(node->child, level, os); // id
                os << " = ";
                rec(node->child->next, level, os); // int
                if (node->next != node->parent->child)
                    os << ',';
                os << std::endl;
                break;
            case ast_var_global:
                rec(node->child, level, os); // type
                os << ' ';
                rec(node->child->next, level, os); // id
                os << ';' << std::endl;
                break;
            case ast_var_param:
                rec(node->child, level, os); // type
                os << ' ';
                rec(node->child->next, level, os); // id
                if (node->next != node->parent->child)
                    os << ", ";
                break;
            case ast_var_local:
                os << std::setfill(' ') << std::setw(level << 2) << "";
                rec(node->child, level, os); // type
                os << ' ';
                rec(node->child->next, level, os); // id
                os << ';' << std::endl;
                break;
            case ast_func: {
                auto _node = node->child;
                rec(_node, level, os); // type
                os << ' ';
                _node = _node->next;
                rec(_node, level, os); // id
                os << '(';
                _node = _node->next;
                rec(_node, level, os); // param
                os << ')' << ' ';
                _node = _node->next;
                rec(_node, level, os); // block
                os << std::endl;
            }
                break;
            case ast_param:
                ast_recursion(node->child, level, os, rec);
                break;
            case ast_block:
                os << '{' << std::endl;
                ast_recursion(node->child, level + 1, os, rec); // stmt
                os << '}';
                break;
            case ast_stmt:
                ast_recursion(node->child, level, os, rec);
                break;
            case ast_exp:
                os << std::setfill(' ') << std::setw(level << 2) << "";
                rec(node->child, level, os); // exp
                os << ';' << std::endl;
                break;
            case ast_sinop:
                if (node->data._op.data == 0) { // 前置
                    os << OP_STRING(node->data._op.op);
                    rec(node->child, level, os); // exp
                } else { // 后置
                    rec(node->child, level, os); // exp
                    os << OP_STRING(node->data._op.op);
                }
                break;
            case ast_binop: {
                auto paran = false;
                switch (node->parent->flag) {
                    case ast_sinop:
                    case ast_binop:
                    case ast_triop:
                        if (node->data._op.op < node->parent->data._op.op)
                            paran = true;
                        break;
                    default:
                        break;
                }
                if (paran)
                    os << '(';
                rec(node->child, level, os); // exp1
                os << ' ' << OP_STRING(node->data._op.op) << ' ';
                rec(node->child->next, level, os); // exp2
                if (paran)
                    os << ')';
            }
                break;
            case ast_triop:
                break;
            case ast_if:
                break;
            case ast_while:
                break;
            case ast_invoke:
                break;
            case ast_id:
                os << node->data._string;
                break;
            case ast_type:
                os << LEX_STRING(node->data._type.type);
                if (node->data._type.ptr > 0) {
                    os << std::setfill('*') << std::setw(node->data._type.ptr) << "";
                }
                break;
            case ast_cast:
                break;
            case ast_string:
                os << '"' << node->data._string << '"';
                break;
            case ast_char:
                os << node->data._char;
                break;
            case ast_uchar:
                os << node->data._uchar;
                break;
            case ast_short:
                os << node->data._short;
                break;
            case ast_ushort:
                os << node->data._ushort;
                break;
            case ast_int:
                os << node->data._int;
                break;
            case ast_uint:
                os << node->data._uint;
                break;
            case ast_long:
                os << node->data._long;
                break;
            case ast_ulong:
                os << node->data._ulong;
                break;
            case ast_float:
                os << node->data._float;
                break;
            case ast_double:
                os << node->data._double;
                break;
        }
    }
}
