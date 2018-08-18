//
// Project: CMiniLang
// Author: bajdcc
//

#include <cassert>
#include <sstream>
#include "cgen.h"
#include "cvm.h"
#include "cast.h"

#define DBG 0

namespace clib {

    string_t class_string_list[] = {
            "NotFound",
            "Enum",
            "Number",
            "Func",
            "Builtin",
            "Global",
            "Param",
            "Local",
    };

    const string_t &class_str(class_t type) {
        assert(type >= clz_not_found && type <= clz_var_local);
        return class_string_list[type];
    }

    // ------------------------------------------

    cgen::cgen(ast_node *node) : root(node) {
        builtin();
        gen();
    }

    void cgen::gen() {
        gen_rec(root);
    }

    void cgen::emit(LEX_T(int) ins) {
        text.push_back(ins);
    }

    void cgen::emit(LEX_T(int) ins, LEX_T(int) op) {
        text.push_back(ins);
        text.push_back(op);
    }

    void cgen::emit_top(LEX_T(int) ins) {
        text.back() = ins;
    }

    void cgen::emit_pop() {
        text.pop_back();
    }

    LEX_T(int) cgen::emit_op(LEX_T(int) ins) {
        text.push_back(ins);
        auto i = index();
        text.push_back(0U);
        return i;
    }

    void cgen::emit_op(LEX_T(int) ins, LEX_T(int) index) {
        text[index] = ins;
    }

    void cgen::emit(ast_node *node) {
        switch ((ast_t) node->flag) {
#define DEFINE_LEXER_STORAGE(t) case ast_##t: \
    emit(IMM, (LEX_T(int))(node->data._##t)); \
    expr_level = LEX_SIZEOF(t); break;
            DEFINE_LEXER_STORAGE(char)
            DEFINE_LEXER_STORAGE(uchar)
            DEFINE_LEXER_STORAGE(short)
            DEFINE_LEXER_STORAGE(ushort)
            DEFINE_LEXER_STORAGE(int)
            DEFINE_LEXER_STORAGE(uint)
            DEFINE_LEXER_STORAGE(float)
#undef DEFINE_LEXER_STORAGE
            case ast_long:
            case ast_ulong:
            case ast_double:
                emit(IMX); // 载入8字节
                emit(node->data._ins._1);
                emit(node->data._ins._2);
                expr_level = 8;
                break;
            case ast_string: {
                auto addr = data.size();
                auto s = node->data._string;
                while (*s) {
                    data.push_back(*s++); // 拷贝字符串至data段
                }
                data.push_back(0);
                auto idx = data.size() % 4;
                if (idx != 0) {
                    idx = 4 - idx;
                    for (auto i = 0; i < idx; ++i) {
                        data.push_back(0); // 对齐
                    }
                }
#if DBG
                printf("[DEBUG] Id::String(\"%s\", %d-%d)\n", cast::display_str(node).c_str(), addr, data.size() - 1);
#endif
                emit(IMM, addr);
                emit(LOAD); // 载入data段指令
                expr_level = 1;
                ptr_level = 1;
            }
                break;
            default:
                assert(!"unsupported type");
                break;
        }
    }

    void cgen::emit(ast_node *node, int ebp) {
        emit(ebp - node->data._int);
    }

    static int align4(int n) {
        if (n % 4 == 0)
            return n;
        return (n | 3) + 1;
    }

    static int size_type(lexer_t type) {
        switch (type) {
#define DEFINE_LEXER_STORAGE(t) case l_##t: return LEX_SIZEOF(t);
            DEFINE_LEXER_STORAGE(char)
            DEFINE_LEXER_STORAGE(uchar)
            DEFINE_LEXER_STORAGE(short)
            DEFINE_LEXER_STORAGE(ushort)
            DEFINE_LEXER_STORAGE(int)
            DEFINE_LEXER_STORAGE(uint)
            DEFINE_LEXER_STORAGE(long)
            DEFINE_LEXER_STORAGE(ulong)
            DEFINE_LEXER_STORAGE(float)
            DEFINE_LEXER_STORAGE(double)
#undef DEFINE_LEXER_STORAGE
            default:
                assert(0);
                break;
        }
    }

    static int size_id(ast_node *node) {
        auto i = 0;
        if (node->prev->data._type.ptr > 0) {
            i = LEX_SIZEOF(ptr);
        } else {
            return size_type(node->prev->data._type.type);
        }
        return i;
    }

    static int size_inc(int expr, int ptr) {
        if (ptr == 0)
            return 1;
        if (ptr == 1) {
            return expr;
        }
        return LEX_SIZEOF(ptr);
    }

    void cgen::emit_deref() {
        if (ptr_level > 1)
            emit(LI);
        else if (ptr_level == 1)
            switch (expr_level) {
                case 1:
                    emit(LC);
                    break;
                case 4:
                    emit(LI);
                    break;
                default:
                    assert(!"unsupported type");
                    break;
            }
        else
            emit(LI);
    }

    void cgen::emitl(ast_node *node) {
        auto n = size_id(node);
        switch (n) {
            case 1:
                emit(LC);
                break;
            case 4:
                emit(LI);
                break;
            default:
                assert(!"unsupported type");
                break;
        }
    }

    void cgen::emits(ins_t ins) {
        switch (ins) {
            case LC:
                emit(SC);
                break;
            case LI:
                emit(SI);
                break;
            default:
                assert(!"unsupported type");
                break;
        }
    }

    LEX_T(int) cgen::index() const {
        return text.size();
    }

    void cgen::calc_level(ast_node *node) {
        ptr_level = node->prev->data._type.ptr;
        switch (node->prev->data._type.type) {
#define DEFINE_LEXER_STORAGE(t) case l_##t: expr_level = LEX_SIZEOF(t); break;
            DEFINE_LEXER_STORAGE(char)
            DEFINE_LEXER_STORAGE(uchar)
            DEFINE_LEXER_STORAGE(short)
            DEFINE_LEXER_STORAGE(ushort)
            DEFINE_LEXER_STORAGE(int)
            DEFINE_LEXER_STORAGE(uint)
            DEFINE_LEXER_STORAGE(long)
            DEFINE_LEXER_STORAGE(ulong)
            DEFINE_LEXER_STORAGE(float)
            DEFINE_LEXER_STORAGE(double)
#undef DEFINE_LEXER_STORAGE
            default:
                assert(0);
                break;
        }
    }

    void cgen::eval() {
        auto entry = symbols[0].find("main");
        if (entry == symbols[0].end()) {
            printf("main() not defined\n");
            throw std::exception();
        }
        cvm vm(text, data);
        vm.exec(entry->second.data);
    }

    void cgen::builtin() {
        symbols.emplace_back(); // global context
        builtin_add("printf", PRTF);
        builtin_add("memcmp", MCMP);
        builtin_add("exit", EXIT);
        builtin_add("memset", MSET);
        builtin_add("open", OPEN);
        builtin_add("read", READ);
        builtin_add("close", CLOS);
        builtin_add("malloc", MALC);
        builtin_add("trace", TRAC);
        builtin_add("trans", TRAN);
    }

    void cgen::builtin_add(const LEX_T(string) &name, ins_t ins) {
        sym_t sym{
                .node = nullptr,
                .clazz = clz_builtin,
                .data = ins
        };
        builtins.insert(std::make_pair(name, sym));
    }

    template<class T>
    static void ast_recursion(ast_node *node, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        do {
            f(i);
            i = i->next;
        } while (i != node);
    }

    void cgen::expect(expect_t type, ast_node *node) {
        switch (type) {
            case expect_non_conflict_id:
                if (conflict_symbol(node->data._string)) {
                    printf("duplicate id: \"%s\"\n", node->data._string);
                    throw std::exception();
                }
                break;
            case expect_valid_id: {
                printf("undefined id: \"%s\"\n", node->data._string);
                throw std::exception();
            }
            case expect_lvalue:
                if (text.back() != LC && text.back() != LI) {
                    std::stringstream ss;
                    cast::print(node, 0, ss);
                    printf("invalid lvalue: \"%s\"\n", ss.str().c_str());
                    throw std::exception();
                }
                break;
            case expect_pointer: {
                std::stringstream ss;
                cast::print(node, 0, ss);
                printf("not a pointer: \"%s\"\n", ss.str().c_str());
                throw std::exception();
            }
        }
    }

    sym_t cgen::find_symbol(const string_t &str) {
        auto f = builtins.find(str);
        if (f != builtins.end()) {
            return f->second;
        }
        for (auto i = symbols.rbegin(); i != symbols.rend(); i++) {
            f = i->find(str);
            if (f != i->end()) {
                return f->second;
            }
        }
        return sym_t{
                .node = nullptr,
                .clazz = clz_not_found,
                .data = 0
        };
    }

    bool cgen::conflict_symbol(const string_t &str) {
        if (builtins.find(str) != builtins.end()) {
            return true;
        }
        if (symbols.back().find(str) != symbols.back().end()) {
            return false;
        }
        for (auto i = symbols.rbegin() + 1; i != symbols.rend(); i++) {
            auto f = i->find(str);
            if (f != i->end()) {
                // 允许变量名覆盖
                if (f->second.clazz == clz_var_local || f->second.clazz == clz_var_global)
                    continue;
                return true;
            }
        }
        return false;
    }

#if DBG
    static string_t type_str(ast_node *node) {
        std::stringstream ss;
        ss << LEX_STRING(node->prev->data._type.type);
        auto n = node->data._type.ptr;
        for (int i = 0; i < n; ++i) {
            ss << '*';
        }
        return ss.str();
    }
#endif

    void cgen::add_symbol(ast_node *node, class_t clazz, LEX_T(int) addr) {
        expect(expect_non_conflict_id, node);
#if DBG
        printf("[DEBUG] Symbol::add(\"%s %s\", %s, %d)\n",
               type_str(node).c_str(), node->data._string,
               CLASS_STRING(clazz).c_str(), addr);
#endif
        sym_t sym{
                .node = node,
                .clazz = clazz,
                .data = addr
        };
        switch (clazz) {
            case clz_enum:
                break;
            case clz_number:
                break;
            case clz_func:
                break;
            case clz_var_global: { // 全局变量，数据段
                auto n = size_id(node);
                sym.data = (int) data.size();
                for (int i = 0; i < n; ++i) {
                    data.push_back(0);
                }
#if DBG
                printf("[DEBUG] Symbol::var_global(used: %d, now: %d)\n", n, data.size());
#endif
            }
                break;
            case clz_var_param: { // 形参，栈
                sym.data = ebp;
                auto n = align4(size_id(node));
                ebp += n;
#if DBG
                printf("[DEBUG] Symbol::var_param(used: %d, now: %d)\n", n, ebp);
#endif
            }
                break;
            case clz_var_local: { // 局部变量，栈
                sym.data = ebp_local;
                auto n = align4(size_id(node));
                ebp_local += n;
#if DBG
                printf("[DEBUG] Symbol::var_local(used: %d, now: %d)\n", n, ebp_local);
#endif
                break;
            }
            default:
                break;
        }
        symbols.back().insert(std::make_pair(node->data._string, sym));
    }

    void cgen::gen_rec(ast_node *node) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n) { this->gen_rec(n); };
        auto type = (ast_t) node->flag;
        switch (type) {
            case ast_root: // 根结点，全局声明
                ast_recursion(node->child, rec);
                break;
            case ast_enum: // 枚举
                ast_recursion(node->child, rec);
                break;
            case ast_enum_unit: // 枚举（等式）
                add_symbol(node->child, clz_enum, node->child->next->data._int);
                break;
            case ast_var_global:
                add_symbol(node->child->next, clz_var_global, 0);
                break;
            case ast_var_param:
                add_symbol(node->child->next, clz_var_param, 0);
                break;
            case ast_var_local:
                add_symbol(node->child->next, clz_var_local, 0);
                break;
            case ast_func: {
                auto _node = node->child; // type
                _node = _node->next; // id
                add_symbol(node->child->next, clz_func, index());
                symbols.emplace_back();
#if DBG
                auto id = _node->data._string;
                printf("[DEBUG] Func::enter(\"%s\")\n", id);
#endif
                ebp = 0;
                _node = _node->next; // param
                rec(_node);
                ebp += 4;
                ebp_local = ebp;
                _node = _node->next; // block
                rec(_node);
#if DBG
                printf("[DEBUG] Func::leave(\"%s\")\n", id);
#endif
                emit(LEV);
                symbols.pop_back();
            }
                break;
            case ast_param:
                ast_recursion(node->child, rec);
                break;
            case ast_block:
                symbols.emplace_back();
#if DBG
                printf("[DEBUG] Block::enter\n");
#endif
                ast_recursion(node->child, rec);
#if DBG
                printf("[DEBUG] Block::leave\n");
#endif
                symbols.pop_back();
                break;
            case ast_stmt:
                ast_recursion(node->child, rec);
                break;
            case ast_return:
#if DBG
                printf("[DEBUG] Func::return\n");
#endif
                if (node->child != nullptr) {
                    rec(node->child);
                }
                emit(LEV);
                break;
            case ast_exp:
                rec(node->child);
                break;
            case ast_exp_param:
                rec(node->child);
                emit(PUSH);
                break;
            case ast_sinop:
                if (node->data._op.data == 0) { // 前置
                    switch (node->data._op.op) {
                        case op_plus:
                            rec(node->child);
                            break;
                        case op_minus:
                            emit(IMM, -1);
                            emit(PUSH);
                            rec(node->child);
                            emit(MUL);
                            break;
                        case op_plus_plus:
                        case op_minus_minus: {
                            rec(node->child); // lvalue
                            auto _expr = expr_level;
                            auto _ptr = ptr_level;
                            auto i = text.back();
                            expect(expect_lvalue, node->child); // 验证左值
                            emit_top(PUSH); // 改载入指令为压栈指令，将左值地址压栈
                            emit(i); // 取出左值
                            emit(PUSH); // 压入左值
                            emit(IMM, size_inc(_expr, _ptr));
                            emit(OP_INS(node->data._op.op));
                            emits((ins_t) i); // 存储指令
                        }
                            break;
                        case op_logical_not:
                            rec(node->child); // lvalue
                            emit(PUSH);
                            emit(IMM, 0);
                            emit(EQ);
                            break;
                        case op_bit_not:
                            rec(node->child); // lvalue
                            emit(PUSH);
                            emit(IMM, -1);
                            emit(XOR);
                            break;
                        case op_bit_and: // 取地址
                            rec(node->child); // lvalue
                            expect(expect_lvalue, node->child); // 验证左值
                            emit_pop(); // 去掉一个读取指令
                            ptr_level++;
                            break;
                        case op_times: // 解引用
                            emit_deref();
                            if (ptr_level > 0)
                                ptr_level--;
                            break;
                        default:
                            assert(!"unsupported op");
                            break;
                    }
                } else { // 后置
                    switch (node->data._op.op) {
                        case op_plus_plus:
                        case op_minus_minus: {
                            rec(node->child); // lvalue
                            auto _expr = expr_level;
                            auto _ptr = ptr_level;
                            auto i = text.back();
                            expect(expect_lvalue, node->child); // 验证左值
                            emit_top(PUSH); // 改载入指令为压栈指令，将左值地址压栈
                            emit(i); // 取出左值
                            emit(PUSH); // 压入左值
                            emit(IMM, size_inc(_expr, _ptr)); // 增量到ax
                            emit(OP_INS(node->data._op.op));
                            emits((ins_t) i); // 存储指令，变量未修改
                            emit(PUSH); // 压入当前值
                            emit(IMM, size_inc(_expr, _ptr)); // 增量到ax
                            emit(OP_INS(node->data._op.op)); // 仅修改ax
                        }
                            break;
                        default:
                            assert(!"unsupported op");
                            break;
                    }
                }
                break;
            case ast_binop: {
                if (node->data._op.op == op_lsquare) {
                    rec(node->child); // exp
                    auto _expr = expr_level;
                    auto _ptr = ptr_level;
                    if (_ptr == 0)
                        expect(expect_pointer, node->child);
                    emit(PUSH); // 压入数组地址
                    rec(node->child->next); // index
                    auto n = size_inc(_expr, _ptr);
                    if (n > 1) {
                        emit(PUSH);
                        emit(IMM, n);
                        emit(MUL);
                        emit(ADD);
                        emit(LI);
                    } else {
                        emit(ADD);
                        emit(LC);
                    }
                    expr_level = _expr;
                    ptr_level = _ptr - 1;
                } else { // 二元运算
                    switch (node->data._op.op) {
                        case op_equal:
                        case op_plus:
                        case op_minus:
                        case op_times:
                        case op_divide:
                        case op_bit_and:
                        case op_bit_or:
                        case op_bit_xor:
                        case op_mod:
                        case op_less_than:
                        case op_less_than_or_equal:
                        case op_greater_than:
                        case op_greater_than_or_equal:
                        case op_not_equal:
                        case op_left_shift:
                        case op_right_shift: {
                            rec(node->child); // exp1
                            auto _expr = expr_level;
                            auto _ptr = ptr_level;
                            emit(PUSH);
                            rec(node->child->next); // exp2
                            auto _expr2 = expr_level;
                            auto _ptr2 = ptr_level;
                            emit(OP_INS(node->data._op.op));
                            // 后面会详细做静态分析
                            expr_level = std::max(_expr, _expr2);
                            ptr_level = std::max(_ptr, _ptr2);
                        }
                            break;
                        case op_assign: {
                            rec(node->child); // lvalue
                            auto _expr = expr_level;
                            auto _ptr = ptr_level; // 保存静态分析类型
                            auto i = text.back();
                            expect(expect_lvalue, node->child); // 验证左值
                            emit_top(PUSH); // 改载入指令为压栈指令，将左值地址压栈
                            rec(node->child->next); // rvalue
                            emits((ins_t) i); // 存储指令
                            expr_level = _expr;
                            ptr_level = _ptr; // 还原静态分析类型
                        }
                            break;
                        case op_plus_assign:
                        case op_minus_assign:
                        case op_times_assign:
                        case op_div_assign:
                        case op_and_assign:
                        case op_or_assign:
                        case op_xor_assign:
                        case op_mod_assign:
                        case op_left_shift_assign:
                        case op_right_shift_assign: {
                            rec(node->child); // lvalue
                            auto _expr = expr_level;
                            auto _ptr = ptr_level; // 保存静态分析类型
                            auto i = text.back();
                            expect(expect_lvalue, node->child); // 验证左值
                            emit_top(PUSH); // 改载入指令为压栈指令，将左值地址压栈
                            emit(i); // 取出左值
                            emit(PUSH);
                            rec(node->child->next); // rvalue
                            emit(OP_INS(node->data._op.op)); // 进行二元操作
                            emits((ins_t) i); // 存储指令
                            expr_level = _expr;
                            ptr_level = _ptr; // 还原静态分析类型
                        }
                            break;
                        default:
                            break;
                    }
                }
            }
                break;
            case ast_triop:
                if (node->data._op.op == op_query) {
                    rec(node->child); // cond
                    auto a = emit_op(JZ);
                    rec(node->child->next); // true
                    auto b = emit_op(JMP);
                    emit_op(index(), a);
                    rec(node->child->prev); // false
                    emit_op(index(), b);
                }
                break;
            case ast_if: {
                //
                // if (...) <statement> [else <statement>]
                //
                //   if (...)           <cond>
                //                      JZ a
                //     <statement>      <statement>
                //   else:              JMP b
                // a:
                //     <statement>      <statement>
                // b:                   b:
                //
                rec(node->child); // if exp
                if (node->child->next == node->child->prev) { // 没有else
                    auto b = emit_op(JZ); // JZ b 条件不满足时跳转到出口
                    rec(node->child->next); // if stmt
                    emit_op(index(), b); // b = 出口
                } else { // 有else
                    auto a = emit_op(JZ); // JZ a 条件不满足时跳转到else块之前
                    rec(node->child->next); // if stmt
                    auto _else = node->child->prev;
                    auto b = emit_op(JMP); // b = true stmt，条件true时运行至此，到达出口
                    emit_op(index(), a); // JZ a 跳转至此，为else块之前
                    rec(_else); // else stmt
                    emit_op(index(), b); // b = 出口
                }
            }
                break;
            case ast_while: {
                //
                // a:                     a:
                //    while (<cond>)        <cond>
                //                          JZ b
                //     <statement>          <statement>
                //                          JMP a
                // b:                     b:
                //
                auto a = index(); // a = 循环起始
                rec(node->child); // cond
                auto b = emit_op(JZ); // JZ b
                rec(node->child->next); // true stmt
                emit(JMP, a); // JMP a
                emit_op(index(), b); // b = 出口
                rec(node->child);
            }
                break;
            case ast_invoke: {
                auto sym = find_symbol(node->data._string);
#if DBG
                printf("[DEBUG] Id::Invoke(\"%s\", %s)\n", node->data._string, CLASS_STRING(sym.clazz).c_str());
#endif
                if (sym.clazz == clz_func) { // 定义的函数
                    ast_recursion(node->child, rec); // param
                    emit(CALL, sym.data); // call func addr
                } else if (sym.clazz == clz_builtin) { // 内建函数
                    ast_recursion(node->child, rec); // param
                    emit((ins_t) sym.data); // builtin-inst
                } else { // 非法
                    expect(expect_valid_id, node);
                }
                auto n = cast::children_size(node); // param count
                if (n > 0) { // 清除参数
                    emit(ADJ, n);
                }
            }
                break;
            case ast_empty: {
                if (node->data._int == 1) {
#if DBG
                    printf("[DEBUG] Func::----\n");
#endif
                    emit(ENT, ebp_local - ebp);
                }
            }
                break;
            case ast_id: {
                auto sym = find_symbol(node->data._string);
                switch (sym.clazz) {
                    case clz_enum: // 枚举
                        emit(IMM, sym.data); // 替换成常量
                        expr_level = 4;
                        ptr_level = 0;
#if DBG
                        printf("[DEBUG] Id::Enum(\"%s\", %s, IMM %d)\n", node->data._string,
                                CLASS_STRING(sym.clazz).c_str(), sym.data);
#endif
                        break;
                    case clz_var_global:
                        emit(IMM, sym.data);
                        emit(LOAD); // 载入data段数据
                        emitl(sym.node);
                        calc_level(sym.node); // 静态分析类型
#if DBG
                        printf("[DEBUG] Id::Global(\"%s\", %s, LOAD %d)\n", node->data._string,
                                CLASS_STRING(sym.clazz).c_str(), sym.data);
#endif
                        break;
                    case clz_var_param:
                    case clz_var_local:
                        emit(LEA, ebp - sym.data);
                        emitl(sym.node);
                        calc_level(sym.node); // 静态分析类型
#if DBG
                        printf("[DEBUG] Id::Local(\"%s\", %s, LEA %d)\n", node->data._string,
                                CLASS_STRING(sym.clazz).c_str(), ebp - sym.data);
#endif
                        break;
                    default: // 非法
                        expect(expect_valid_id, node);
                        break;
                }
            }
                break;
            case ast_type:
                break;
            case ast_cast:
                rec(node->child);
                expr_level = node->data._type.type; // 修正静态分析类型
                ptr_level = node->data._type.ptr;
                break;
            case ast_char:
            case ast_uchar:
            case ast_short:
            case ast_ushort:
            case ast_int:
            case ast_uint:
            case ast_long:
            case ast_ulong:
            case ast_float:
            case ast_double:
            case ast_string:
                emit(node);
                break;
        }
    }
}
