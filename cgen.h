//
// Project: CMiniLang
// Author: bajdcc
//

#ifndef CMINILANG_GEN_H
#define CMINILANG_GEN_H

#include <map>
#include <memory>
#include <vector>
#include "types.h"
#include "memory.h"
#include "cast.h"

namespace clib {

    // instructions

    enum ins_t {
        NOP, LEA, IMM, IMX, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, SI, LC, SC, PUSH, LOAD,
        OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
        OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, TRAC, TRAN, EXIT
    };

    enum class_t {
        clz_not_found,
        clz_enum,
        clz_number,
        clz_func,
        clz_builtin,
        clz_var_global,
        clz_var_param,
        clz_var_local,
    };

    extern string_t class_string_list[];
#define CLASS_STRING(t) class_str(t)
    const string_t &class_str(class_t);

    enum expect_t {
        expect_non_conflict_id,
        expect_valid_id,
        expect_lvalue,
        expect_pointer,
    };

    struct sym_t {
        ast_node *node;
        class_t clazz;
        int data;
    };

    class cgen {
    public:
        explicit cgen(ast_node *node);
        ~cgen() = default;

        void eval();

    private:
        void gen();
        void gen_rec(ast_node *node);

        void emit(LEX_T(int));
        void emit(LEX_T(int), LEX_T(int));
        void emit_top(LEX_T(int));
        void emit_pop();
        LEX_T(int) emit_op(LEX_T(int));
        void emit_op(LEX_T(int), LEX_T(int) index);
        void emit(ast_node *node);
        void emit(ast_node *node, int ebp);
        void emit_deref();
        void emitl(ast_node *node);
        void emits(ins_t ins);

        LEX_T(int) index() const;

        void expect(expect_t type, ast_node *node);
        sym_t find_symbol(const string_t &str);
        bool conflict_symbol(const string_t &str);
        void add_symbol(ast_node *node, class_t clazz, LEX_T(int) addr);

        void calc_level(ast_node *node);

    private:
        void builtin();
        void builtin_add(const LEX_T(string) &name, ins_t value);

    private:
        ast_node *root;
        int ebp{0};
        int ebp_local{0};
        int expr_level{0};
        int ptr_level{0};
        std::vector<LEX_T(int)> text; // 代码
        std::vector<LEX_T(char)> data; // 数据
        std::vector<std::unordered_map<LEX_T(string), sym_t>> symbols;
        std::unordered_map<LEX_T(string), sym_t> builtins;
    };
}

#endif //CMINILANG_GEN_H
