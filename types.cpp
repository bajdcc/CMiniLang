//
// Project: CMiniLang
// Author: bajdcc
//

#include <cassert>
#include "types.h"

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

    std::tuple<operator_t, string_t, string_t> operator_string_list[] = {
        std::make_tuple(op__start, "@START", "@START"),
        std::make_tuple(op_assign, "=", "assign"),
        std::make_tuple(op_equal, "==", "equal"),
        std::make_tuple(op_plus, "+", "plus"),
        std::make_tuple(op_plus_assign, "+=", "plus_assign"),
        std::make_tuple(op_minus, "-", "minus"),
        std::make_tuple(op_minus_assign, "-=", "minus_assign"),
        std::make_tuple(op_times, "*", "times"),
        std::make_tuple(op_times_assign, "*=", "times_assign"),
        std::make_tuple(op_divide, "/", "divide"),
        std::make_tuple(op_div_assign, "/=", "div_assign"),
        std::make_tuple(op_bit_and, "&", "bit_and"),
        std::make_tuple(op_and_assign, "&=", "and_assign"),
        std::make_tuple(op_bit_or, "|", "bit_or"),
        std::make_tuple(op_or_assign, "|=", "or_assign"),
        std::make_tuple(op_bit_xor, "^", "bit_xor"),
        std::make_tuple(op_xor_assign, "^=", "xor_assign"),
        std::make_tuple(op_mod, "%", "mod"),
        std::make_tuple(op_mod_assign, "%=", "mod_assign"),
        std::make_tuple(op_less_than, "<", "less_than"),
        std::make_tuple(op_less_than_or_equal, "<=", "less_than_or_equal"),
        std::make_tuple(op_greater_than, ">", "greater_than"),
        std::make_tuple(op_greater_than_or_equal, ">=", "greater_than_or_equal"),
        std::make_tuple(op_logical_not, "!", "logical_not"),
        std::make_tuple(op_not_equal, "!=", "not_equal"),
        std::make_tuple(op_escape, "\\", "escape"),
        std::make_tuple(op_query, "?", "query"),
        std::make_tuple(op_bit_not, "~", "bit_not"),
        std::make_tuple(op_lparan, "(", "lparan"),
        std::make_tuple(op_rparan, ")", "rparan"),
        std::make_tuple(op_lbrace, "{", "lbrace"),
        std::make_tuple(op_rbrace, "}", "rbrace"),
        std::make_tuple(op_lsquare, "[", "lsquare"),
        std::make_tuple(op_rsquare, "]", "rsquare"),
        std::make_tuple(op_comma, ",", "comma"),
        std::make_tuple(op_dot, ".", "dot"),
        std::make_tuple(op_semi, ";", "semi"),
        std::make_tuple(op_colon, ":", "colon"),
        std::make_tuple(op_plus_plus, "++", "plus_plus"),
        std::make_tuple(op_minus_minus, "--", "minus_minus"),
        std::make_tuple(op_logical_and, "&&", "logical_and"),
        std::make_tuple(op_logical_or, "||", "logical_or"),
        std::make_tuple(op_pointer, "->", "pointer"),
        std::make_tuple(op_left_shift, "<<", "left_shift"),
        std::make_tuple(op_right_shift, ">>", "right_shift"),
        std::make_tuple(op_left_shift_assign, "<<=", "left_shift_assign"),
        std::make_tuple(op_right_shift_assign, ">>=", "right_shift_assign"),
        std::make_tuple(op_ellipsis, "...", "ellipsis"),
        std::make_tuple(op__end, "@END", "@END"),
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

    int op_pred[] = {
        9999, // op__start,
        1401, // op_assign,
        401, // op_plus,
        402, // op_minus,
        302, // op_times,
        301, // op_divide,
        9000, // op_escape,
        1301, // op_query,
        303, // op_mod,
        801, // op_bit_and,
        1001, // op_bit_or,
        208, // op_bit_not,
        901, // op_bit_xor,
        207, // op_logical_not,
        603, // op_less_than,
        601, // op_greater_than,
        102, // op_lparan,
        102, // op_rparan,
        9000, // op_lbrace,
        9000, // op_rbrace,
        101, // op_lsquare,
        101, // op_rsquare,
        1501, // op_comma,
        103, // op_dot,
        9000, // op_semi,
        1302, // op_colon,
        701, // op_equal,
        702, // op_not_equal,
        203, // op_plus_plus,
        204, // op_minus_minus,
        1405, // op_plus_assign,
        1406, // op_minus_assign,
        1403, // op_times_assign,
        1402, // op_div_assign,
        1409, // op_and_assign,
        1411, // op_or_assign,
        1410, // op_xor_assign,
        1404, // op_mod_assign,
        604, // op_less_than_or_equal,
        602, // op_greater_than_or_equal,
        1101, // op_logical_and,
        1201, // op_logical_or,
        104, // op_pointer,
        501, // op_left_shift,
        502, // op_right_shift,
        1407, // op_left_shift_assign,
        1408, // op_right_shift_assign,
        9000, // op_ellipsis,
        9999, // op__end
    };

    int lexer_operatorpred(operator_t type) {
        assert(type > op__start && type < op__end);
        return op_pred[type];
    }
}
