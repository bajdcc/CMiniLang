//
// Project: CMiniLang
// Author: bajdcc
//

#include <cassert>
#include "types.h"
#include "cgen.h"

namespace clib {
    string_t lexer_string_list[] = {
        "none",
        "ptr",
        "error",
        "char",
        "uchar",
        "short",
        "ushort",
        "int",
        "uint",
        "long",
        "ulong",
        "float",
        "double",
        "operator",
        "keyword",
        "identifier",
        "string",
        "comment",
        "space",
        "newline",
        "END"
    };

    const string_t &lexer_typestr(lexer_t type) {
        assert(type >= l_none && type < l_end);
        return lexer_string_list[type];
    }

    string_t keyword_string_list[] = {
        "@START",
        "auto",
        "bool",
        "break",
        "case",
        "char",
        "const",
        "continue",
        "default",
        "do",
        "double",
        "else",
        "enum",
        "extern",
        "false",
        "float",
        "for",
        "goto",
        "if",
        "int",
        "long",
        "register",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "struct",
        "switch",
        "true",
        "typedef",
        "union",
        "unsigned",
        "void",
        "volatile",
        "while",
        "@END",
    };

    const string_t &lexer_keywordstr(keyword_t type) {
        assert(type > k__start && type < k__end);
        return keyword_string_list[type - k__start];
    }

    std::tuple<operator_t, string_t, string_t, ins_t, int> operator_string_list[] = {
        std::make_tuple(op__start, "@START", "@START", NOP, 9999),
        std::make_tuple(op_assign, "=", "assign", NOP, 1401),
        std::make_tuple(op_equal, "==", "equal", EQ, 701),
        std::make_tuple(op_plus, "+", "plus", ADD, 401),
        std::make_tuple(op_plus_assign, "+=", "plus_assign", ADD, 1405),
        std::make_tuple(op_minus, "-", "minus", SUB, 402),
        std::make_tuple(op_minus_assign, "-=", "minus_assign", SUB, 1406),
        std::make_tuple(op_times, "*", "times", MUL, 302),
        std::make_tuple(op_times_assign, "*=", "times_assign", MUL, 1403),
        std::make_tuple(op_divide, "/", "divide", DIV, 301),
        std::make_tuple(op_div_assign, "/=", "div_assign", DIV, 1402),
        std::make_tuple(op_bit_and, "&", "bit_and", AND, 801),
        std::make_tuple(op_and_assign, "&=", "and_assign", AND, 1409),
        std::make_tuple(op_bit_or, "|", "bit_or", OR, 1001),
        std::make_tuple(op_or_assign, "|=", "or_assign", OR, 1411),
        std::make_tuple(op_bit_xor, "^", "bit_xor", XOR, 901),
        std::make_tuple(op_xor_assign, "^=", "xor_assign", XOR, 1410),
        std::make_tuple(op_mod, "%", "mod", MOD, 303),
        std::make_tuple(op_mod_assign, "%=", "mod_assign", MOD, 1404),
        std::make_tuple(op_less_than, "<", "less_than", LT, 603),
        std::make_tuple(op_less_than_or_equal, "<=", "less_than_or_equal", LE, 604),
        std::make_tuple(op_greater_than, ">", "greater_than", GT, 601),
        std::make_tuple(op_greater_than_or_equal, ">=", "greater_than_or_equal", GE, 602),
        std::make_tuple(op_logical_not, "!", "logical_not", NOP, 207),
        std::make_tuple(op_not_equal, "!=", "not_equal", NE, 702),
        std::make_tuple(op_escape, "\\", "escape", NOP, 9000),
        std::make_tuple(op_query, "?", "query", NOP, 1301),
        std::make_tuple(op_bit_not, "~", "bit_not", NOP, 208),
        std::make_tuple(op_lparan, "(", "lparan", NOP, 102),
        std::make_tuple(op_rparan, ")", "rparan", NOP, 102),
        std::make_tuple(op_lbrace, "{", "lbrace", NOP, 9000),
        std::make_tuple(op_rbrace, "}", "rbrace", NOP, 9000),
        std::make_tuple(op_lsquare, "[", "lsquare", NOP, 101),
        std::make_tuple(op_rsquare, "]", "rsquare", NOP, 101),
        std::make_tuple(op_comma, ",", "comma", NOP, 1501),
        std::make_tuple(op_dot, ".", "dot", NOP, 103),
        std::make_tuple(op_semi, ";", "semi", NOP, 9000),
        std::make_tuple(op_colon, ":", "colon", NOP, 1302),
        std::make_tuple(op_plus_plus, "++", "plus_plus", ADD, 203),
        std::make_tuple(op_minus_minus, "--", "minus_minus", SUB, 204),
        std::make_tuple(op_logical_and, "&&", "logical_and", JZ, 1101),
        std::make_tuple(op_logical_or, "||", "logical_or", JNZ, 1201),
        std::make_tuple(op_pointer, "->", "pointer", NOP, 104),
        std::make_tuple(op_left_shift, "<<", "left_shift", SHL, 501),
        std::make_tuple(op_right_shift, ">>", "right_shift", SHR, 502),
        std::make_tuple(op_left_shift_assign, "<<=", "left_shift_assign", SHL, 1407),
        std::make_tuple(op_right_shift_assign, ">>=", "right_shift_assign", SHR, 1408),
        std::make_tuple(op_ellipsis, "...", "ellipsis", NOP, 9000),
        std::make_tuple(op__end, "@END", "@END", NOP, 9999),
    };

    const string_t &lexer_opstr(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<1>(operator_string_list[type]);
    }

    const string_t &lexer_opnamestr(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<2>(operator_string_list[type]);
    }

    string_t err_string_list[] = {
        "@START",
        "#E !char!",
        "#E !operator!",
        "#E !comment!",
        "#E !digit!",
        "#E !string!",
        "@END",
    };

    const string_t &lexer_errstr(error_t type) {
        assert(type > e__start && type < e__end);
        return err_string_list[type];
    }

    int lexer_operatorpred(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<4>(operator_string_list[type]);
    }

    int lexer_op2ins(operator_t type) {
        assert(type > op__start && type < op__end);
        return std::get<3>(operator_string_list[type]);
    }
}
