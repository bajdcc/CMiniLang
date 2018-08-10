//
// Project: CMiniLang
// Author: bajdcc
//

#include <cassert>
#include <climits>
#include "clexer.h"

namespace clib {

    clexer::clexer(string_t str) : str(str) {
        length = str.length();
        assert(length > 0);
        initMap();
    }

    clexer::~clexer() = default;

#define DEFINE_LEXER_GETTER(t) \
LEX_T(t) clexer::get_##t() const \
{ \
    return bags._##t; \
}

    DEFINE_LEXER_GETTER(char)
    DEFINE_LEXER_GETTER(uchar)
    DEFINE_LEXER_GETTER(short)
    DEFINE_LEXER_GETTER(ushort)
    DEFINE_LEXER_GETTER(int)
    DEFINE_LEXER_GETTER(uint)
    DEFINE_LEXER_GETTER(long)
    DEFINE_LEXER_GETTER(ulong)
    DEFINE_LEXER_GETTER(float)
    DEFINE_LEXER_GETTER(double)
    DEFINE_LEXER_GETTER(operator)
    DEFINE_LEXER_GETTER(keyword)
    DEFINE_LEXER_GETTER(identifier)
    DEFINE_LEXER_GETTER(string)
    DEFINE_LEXER_GETTER(comment)
    DEFINE_LEXER_GETTER(space)
    DEFINE_LEXER_GETTER(newline)
    DEFINE_LEXER_GETTER(error)
#undef DEFINE_LEXER_GETTER

#define DEFINE_LEXER_GETTER(t) \
LEX_T(t) clexer::get_store_##t(int index) const \
{ \
    return storage._##t[index]; \
}

    DEFINE_LEXER_GETTER(char)
    DEFINE_LEXER_GETTER(uchar)
    DEFINE_LEXER_GETTER(short)
    DEFINE_LEXER_GETTER(ushort)
    DEFINE_LEXER_GETTER(int)
    DEFINE_LEXER_GETTER(uint)
    DEFINE_LEXER_GETTER(long)
    DEFINE_LEXER_GETTER(ulong)
    DEFINE_LEXER_GETTER(float)
    DEFINE_LEXER_GETTER(double)
    DEFINE_LEXER_GETTER(identifier)
    DEFINE_LEXER_GETTER(string)
#undef DEFINE_LEXER_GETTER

    bool match_pred(smatch_t::value_type sm) {
        return sm.matched;
    }

    lexer_t clexer::record_error(error_t error, int skip) {
        err_record_t err{};
        err.line = line;
        err.column = column;
        err.start_idx = index;
        err.end_idx = index + skip;
        err.err = error;
        err.str = str.substr(err.start_idx, err.end_idx - err.start_idx);
        records.push_back(err);
        bags._error = error;
        move(skip);
        return l_error;
    }

    const clexer::err_record_t &clexer::recent_error() const {
        return records.back();
    }

    lexer_t clexer::expect(int start, error_t error, const regex_t &re, int skip) {
        if (std::regex_search(str.cbegin() + index + start, str.cend(), sm, re)) { // handle error
            if (sm[0].matched) {
                auto ml = sm[0].length();
                return record_error(error, ml);
            }
        }
        move(skip); // move to end
        return record_error(error, skip);
    }

    lexer_t clexer::next() {
        auto c = local();
        if (c == -1) {
            type = l_end;
            return l_end;
        }
        type = l_error;
        if (isalpha(c) || c == '_') { // 变量名或关键字
            type = next_alpha();
        } else if (isdigit(c)) { // 数字
            type = next_digit();
        } else if (isspace(c)) { // 空白字符
            type = next_space();
        } else if (c == '\'') { // 字符
            type = next_char();
        } else if (c == '\"') { // 字符串
            type = next_string();
        } else if (c == '/') { // 注释
            auto c2 = local(1);
            if (c2 == '/' || c2 == '*') { // 注释
                type = next_comment();
            } else { // 操作符
                type = next_operator();
            }
        } else {
            type = next_operator();
        }
        return type;
    }

    lexer_t clexer::get_type() const {
        return type;
    }

    int clexer::get_line() const {
        return line;
    }

    int clexer::get_column() const {
        return column;
    }

    int clexer::get_last_line() const {
        return last_line;
    }

    int clexer::get_last_column() const {
        return last_column;
    }

    string_t clexer::current() const {
        switch (type) {
            case l_space:
            case l_newline:
            case l_comment:
                return "...\t[" + LEX_STRING(type) + "]";
            case l_operator:
                return str.substr(last_index, index - last_index) + "\t[" + OPERATOR_STRING(bags._operator) + "]";
            default:
                break;
        }
        return str.substr(last_index, index - last_index);
    }

    std::string clexer::store_start() {
        std::stringstream ss;
        while (next() != l_end) {
            auto s = store();
            if (!s.empty()) {
                ss << s;
                ss.put(' ');
            }
        }
        return ss.str();
    }

    string_t clexer::store() {
        static char buf[52];
        switch (type) {
            case l_none:
            case l_error:
            case l_comment:
            case l_space:
            case l_newline:
                return "";
#define DEFINE_LEXER_STORAGE_GET(t, a) case l_##t: { \
        itoa(storage._##t.size(), buf + 1, 10); \
        storage._##t.push_back(get_##t()); \
        buf[0] = a; \
        return buf; \
    }
            DEFINE_LEXER_STORAGE_GET(char, 'c')
            DEFINE_LEXER_STORAGE_GET(uchar, 'C')
            DEFINE_LEXER_STORAGE_GET(short, 's')
            DEFINE_LEXER_STORAGE_GET(ushort, 'S')
            DEFINE_LEXER_STORAGE_GET(int, 'i')
            DEFINE_LEXER_STORAGE_GET(uint, 'I')
            DEFINE_LEXER_STORAGE_GET(long, 'l')
            DEFINE_LEXER_STORAGE_GET(ulong, 'L')
            DEFINE_LEXER_STORAGE_GET(float, 'f')
            DEFINE_LEXER_STORAGE_GET(double, 'd')
            DEFINE_LEXER_STORAGE_GET(string, 's')
            DEFINE_LEXER_STORAGE_GET(identifier, 't')
#undef DEFINE_LEXER_STORAGE_GET
#define DEFINE_LEXER_STORAGE_GET(t, a) case l_##t: { \
        itoa((int)get_##t(), buf + 1, 10); \
        buf[0] = a; \
        return buf; \
    }
            DEFINE_LEXER_STORAGE_GET(keyword, 'k')
#undef DEFINE_LEXER_STORAGE_GET
            case l_operator:
                return str.substr(last_index, index - last_index);
            case l_end:
                break;
            default:
                break;
        }
        return "";
    }

    bool clexer::is_type(lexer_t type) const {
        return get_type() == type;
    }

    bool clexer::is_keyword(keyword_t type) const {
        return get_type() == l_keyword && get_keyword() == type;
    }

    bool clexer::is_operator(operator_t type) const {
        return get_type() == l_operator && get_operator() == type;
    }

    bool clexer::is_operator(operator_t type1, operator_t type2) const {
        return get_type() == l_operator && (get_operator() == type1 || get_operator() == type2);
    }

    bool clexer::is_number() const {
        return get_type() >= l_char && get_type() <= l_double;
    }

    bool clexer::is_integer() const {
        return get_type() >= l_char && get_type() <= l_long;
    }

    bool clexer::is_basetype() const {
        if (get_type() != l_keyword)
            return false;
        switch (get_keyword()) {
#define DEFINE_LEXER_CASE(t) case k_##t:
            DEFINE_LEXER_CASE(char)
            DEFINE_LEXER_CASE(short)
            DEFINE_LEXER_CASE(int)
            DEFINE_LEXER_CASE(long)
            DEFINE_LEXER_CASE(unsigned)
                return true;
#undef DEFINE_LEXER_CASE
            default:
                break;
        }
        return false;
    }

    LEX_T(uint) clexer::get_integer() const {
        assert(is_integer());
        switch (type) {
#define DEFINE_LEXER_CASE(t) case l_##t: return get_##t();
            DEFINE_LEXER_CASE(char)
            DEFINE_LEXER_CASE(uchar)
            DEFINE_LEXER_CASE(short)
            DEFINE_LEXER_CASE(ushort)
            DEFINE_LEXER_CASE(int)
            DEFINE_LEXER_CASE(uint)
            DEFINE_LEXER_CASE(long)
            DEFINE_LEXER_CASE(ulong)
#undef DEFINE_LEXER_CASE
            default:
                break;
        }
        return 0;
    }

    LEX_T(int) clexer::get_sizeof() const {
        assert(is_type(l_keyword));
        switch (get_keyword()) {
#define DEFINE_LEXER_KEYWORD(t) case k_##t: return LEX_SIZEOF(t);
            DEFINE_LEXER_KEYWORD(char)
            DEFINE_LEXER_KEYWORD(short)
            DEFINE_LEXER_KEYWORD(int)
            DEFINE_LEXER_KEYWORD(long)
            DEFINE_LEXER_KEYWORD(float)
            DEFINE_LEXER_KEYWORD(double)
#undef DEFINE_LEXER_KEYWORD
            default:
                assert(!"unsupported type");
                break;
        }
        return -1;
    }

    lexer_t clexer::get_typeof(bool _unsigned) const {
        assert(is_type(l_keyword));
        switch (get_keyword()) {
#define DEFINE_LEXER_KEYWORD(t) case k_##t: return lexer_t(l_##t + int(_unsigned));
            DEFINE_LEXER_KEYWORD(char)
            DEFINE_LEXER_KEYWORD(short)
            DEFINE_LEXER_KEYWORD(int)
            DEFINE_LEXER_KEYWORD(long)
            DEFINE_LEXER_KEYWORD(float)
            DEFINE_LEXER_KEYWORD(double)
#undef DEFINE_LEXER_KEYWORD
            case k_void:
                return l_none;
            default:
                assert(!"unsupported type");
                break;
        }
        return l_error;
    }

    void clexer::move(int idx, int inc, bool newline) {
        last_index = index;
        last_line = line;
        last_column = column;
        if (newline) {
            column = 1;
            line += inc;
        } else {
            if (inc < 0)
                column += idx;
            else
                column += inc;
        }
        index += idx;
    }

    // 计算幂
    template<class T>
    static T calc_exp(T d, int e) {
        if (e == 0)
            return d;
        else if (e > 0)
            for (int i = 0; i < e; i++)
                d *= 10;
        else
            for (int i = e; i < 0; i++)
                d /= 10;
        return d;
    }

    static lexer_t unsigned_type(lexer_t t) {
        switch (t) {
            case l_char:
                return l_uchar;
            case l_short:
                return l_ushort;
            case l_int:
                return l_uint;
            case l_long:
                return l_ulong;
            default:
                return t;
        }
    }

    static lexer_t digit_type_postfix(char c) {
        switch (c) {
            case 'C':
            case 'c':
                return l_char;
            case 'S':
            case 's':
                return l_short;
            case 'I':
            case 'i':
                return l_int;
            case 'L':
            case 'l':
                return l_long;
            case 'F':
            case 'f':
                return l_float;
            case 'D':
            case 'd':
                return l_double;
            default:
                return l_error;
        }
    }

    lexer_t clexer::digit_type(lexer_t t, int i) {
        if (i == length) {
            return l_error;
        }
        if (str[i] == 'U' || str[i] == 'u') {
            if (++i == length) {
                return unsigned_type(t);
            }
            return unsigned_type(digit_type_postfix(str[i]));
        } else {
            if ((t = digit_type_postfix(str[i])) == l_error) {
                return l_error;
            }
            return t;
        }
    }

    bool clexer::digit_from_integer(lexer_t t, LEX_T(ulong) n) {
        switch (t) {
#define DEFINE_LEXER_CONV_INTEGER(t) case l_##t: bags._##t = (LEX_T(t)) n; break;
            DEFINE_LEXER_CONV_INTEGER(char)
            DEFINE_LEXER_CONV_INTEGER(uchar)
            DEFINE_LEXER_CONV_INTEGER(short)
            DEFINE_LEXER_CONV_INTEGER(ushort)
            DEFINE_LEXER_CONV_INTEGER(int)
            DEFINE_LEXER_CONV_INTEGER(uint)
            DEFINE_LEXER_CONV_INTEGER(long)
            DEFINE_LEXER_CONV_INTEGER(ulong)
            DEFINE_LEXER_CONV_INTEGER(float)
            DEFINE_LEXER_CONV_INTEGER(double)
#undef DEFINE_LEXER_CONV_INTEGER
            default:
                return false;
        }
        return true;
    }

    bool clexer::digit_from_double(lexer_t t, LEX_T(double) d) {
        switch (t) {
#define DEFINE_LEXER_CONV_INTEGER(t) case l_##t: bags._##t = (LEX_T(t)) d; break;
            DEFINE_LEXER_CONV_INTEGER(char)
            DEFINE_LEXER_CONV_INTEGER(uchar)
            DEFINE_LEXER_CONV_INTEGER(short)
            DEFINE_LEXER_CONV_INTEGER(ushort)
            DEFINE_LEXER_CONV_INTEGER(int)
            DEFINE_LEXER_CONV_INTEGER(uint)
            DEFINE_LEXER_CONV_INTEGER(long)
            DEFINE_LEXER_CONV_INTEGER(ulong)
            DEFINE_LEXER_CONV_INTEGER(float)
            DEFINE_LEXER_CONV_INTEGER(double)
#undef DEFINE_LEXER_CONV_INTEGER
            default:
                return false;
        }
        return true;
    }

    lexer_t clexer::digit_return(lexer_t t, LEX_T(ulong) n, LEX_T(double) d, int i) {
        if (t == l_int) {
            bags._int = (int) n;
        } else if (t == l_double) {
            bags._double = d;
        } else if (t == l_long) {
            bags._long = n;
        } else {
            bags._double = d;
        }
        move(i - index);
        return t;
    }

    // 参考自：https://github.com/bajdcc/CEval/blob/master/CEval/CEval.cpp#L105
    lexer_t clexer::next_digit() {
        // 假定这里的数字规则是以0-9开头
        // 正则：^((?:\d+(\.)?\d*)(?:[eE][+-]?\d+)?)([uU])?([fFdCcSsDiIlL])?$
        // 手动实现atof/atoi，并类型转换
        // 其他功能：int溢出转double，e科学记数法
        // 注意：这里不考虑负数，因为估计到歧义（可能是减法呢？）
        auto _type = l_int; // 默认是整型
        auto _postfix = l_none;
        auto i = index;
        auto n = 0ULL, _n = 0ULL;
        auto d = 0.0D;
        // 判断整数部分
        for (; i < length && (isdigit(str[i])); i++) { // 解析整数部分
            if (_type == l_double) { // 小数加位，溢出后自动转换
                d *= 10.0;
                d += str[i] - '0';
            } else { // 整数加位
                _n = n;
                n *= 10;
                n += str[i] - '0';
            }
            if (_type == l_int) { // 超过int范围，转为long
                if (n > INT_MAX)
                    _type = l_long;
            } else if (_type == l_long) { // 超过long范围，转为double
                if (n > LONG_LONG_MAX) {
                    d = (double) _n;
                    d *= 10.0;
                    d += str[i] - '0';
                    _type = l_double;
                }
            }
        }
        if (i == length) { // 只有整数部分
            return digit_return(_type, n, d, i);
        }
        if ((_postfix = digit_type(_type, i)) != l_error) {
            move(i - index);
            return digit_from_integer(_postfix, n) ? _postfix : _type;
        }
        if (str[i] == '.') { // 解析小数部分
            auto l = ++i;
            for (; i < length && (isdigit(str[i])); i++) {
                d *= 10.0;
                d += str[i] - '0';
            }
            l = i - l;
            if (l > 0) {
                d = (double) n + calc_exp(d, -l);
                _type = l_double;
            }
        }
        if (i == length) { // 只有整数部分和小数部分
            return digit_return(_type, n, d, i);
        }
        if ((_postfix = digit_type(_type, i)) != l_error) {
            move(i - index);
            if (_type == l_int)
                return digit_from_integer(_postfix, n) ? _postfix : _type;
            else
                return digit_from_double(_postfix, d) ? _postfix : _type;
        }
        if (str[i] == 'e' || str[i] == 'E') { // 科学计数法强制转成double
            auto neg = false;
            auto e = 0;
            if (_type != l_double) {
                _type = l_double;
                d = (double) n;
            }
            if (++i == length) {
                return digit_return(_type, n, d, i);
            }
            if (!isdigit(str[i])) {
                if (str[i] == '-') {
                    if (++i == length)
                        return digit_return(_type, n, d, i);
                    neg = true;
                } else if (str[i] == '+') {
                    if (++i == length)
                        return digit_return(_type, n, d, i);
                } else {
                    return digit_return(_type, n, d, i);
                }
            }
            for (; i < length && (isdigit(str[i])); i++) { // 解析指数部分
                e *= 10;
                e += str[i] - '0';
            }
            d = calc_exp(d, neg ? -e : e);
        }
        if ((_postfix = digit_type(_type, i)) != l_error) {
            move(i - index);
            if (_type == l_int)
                return digit_from_integer(_postfix, n) ? _postfix : _type;
            else
                return digit_from_double(_postfix, d) ? _postfix : _type;
        }
        return digit_return(_type, n, d, i);
    }

    lexer_t clexer::next_alpha() {
        sint i;
        for (i = index + 1; i < length && (isalnum(str[i]) || str[i] == '_'); i++);
        auto s = str.substr(index, i - index);
        auto kw = mapKeyword.find(s);
        if (kw != mapKeyword.end()) { // 关键字
            bags._keyword = kw->second;
            move(s.length());
            return l_keyword;
        }
        // 普通变量名
        bags._identifier = s;
        move(s.length());
        return l_identifier;
    }

    lexer_t clexer::next_space() {
        uint i, j;
        switch (str[index]) {
            case ' ':
            case '\t':
                for (i = index + 1; i < length && (str[i] == ' ' || str[i] == '\t'); i++);
                bags._space = i - index;
                move(bags._space);
                return l_space;
            case '\r':
            case '\n':
                for (i = index, j = 0; i < length &&
                                       (str[i] == '\r' || (str[i] == '\n' ? ++j > 0 : false)); i++);
                bags._newline = j;
                move(i - index, bags._newline, true);
                return l_newline;
        }
        assert(!"space not match"); // cannot reach
        move(1);
        return l_error;
    }

    static int hex2dec(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        } else {
            return -1;
        }
    }

    static int escape(char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else {
            switch (c) { // like \r, \n, ...
                case 'b':
                    return '\b';
                case 'f':
                    return '\f';
                case 'n':
                    return '\n';
                case 'r':
                    return '\r';
                case 't':
                    return '\t';
                case 'v':
                    return '\v';
                case '\'':
                    return '\'';
                case '\"':
                    return '\"';
                case '\\':
                    return '\\';
                default:
                    return -1;
            }
        }
    }

    lexer_t clexer::next_char() {
        sint i;
        for (i = 1; index + i < length && str[index + i] != '\'' && i <= 4; i++);
        if (i == 1) { // ''
            return record_error(e_invalid_char, i + 1);
        }
        auto j = index + i;
        i++;
        if (j < length && str[j] == '\'') {
            if (str[index + 1] == '\\') {
                if (i == 3) { // '\'
                    return record_error(e_invalid_char, i);
                }
                if (i == 4) { // '\?'
                    auto esc = escape(str[index + 2]);
                    if (esc != -1) {
                        bags._char = (char) esc;
                        move(i);
                        return l_char;
                    }
                    return record_error(e_invalid_char, i);
                }
                if (i == 5) { // '\x?'
                    if (str[index + 1] == '\\' && str[index + 2] == 'x') {
                        auto esc = hex2dec(str[index + 3]);
                        if (esc != -1) {
                            bags._char = (char) esc;
                            move(i);
                            return l_char;
                        }
                    }
                    return record_error(e_invalid_char, i);
                }
                // '\x??'
                if (str[index + 1] == '\\' && str[index + 2] == 'x') {
                    auto esc = hex2dec(str[index + 3]);
                    if (esc != -1) {
                        bags._char = (char) esc;
                        esc = hex2dec(str[index + 4]);
                        if (esc != -1) {
                            bags._char *= 0x10;
                            bags._char += (char) esc;
                            move(i);
                            return l_char;
                        }
                    }
                }
                return record_error(e_invalid_char, i);
            } else if (i == 3) { // '?'
                bags._char = str[index + 1];
                move(i);
                return l_char;
            }
        }
        return record_error(e_invalid_char, 1);
    }

    lexer_t clexer::next_string() {
        sint i;
        auto prev = str[index];
        // 寻找非'\"'的第一个'"'
        for (i = 1; index + i < length && (prev == '\\' || (str[index + i]) != '"'); i++, prev = str[index + i]);
        auto j = index + i;
        if (j == length) { // " EOF
            return record_error(e_invalid_string, i);
        }
        std::stringstream ss;
        auto status = 1; // 状态机
        char c = 0;
        for (i = index + 1; i < j;) {
            switch (status) {
                case 1: { // 处理字符
                    if (str[i] == '\\') {
                        status = 2;
                    } else { // '?'
                        ss << str[i];
                    }
                    i++;
                }
                    break;
                case 2: { // 处理转义
                    if (str[i] == 'x') {
                        status = 3;
                        i++;
                    } else {
                        auto esc = escape(str[i]);
                        if (esc != -1) {
                            ss << (char) esc;
                            i++;
                            status = 1;
                        } else {
                            status = 0; // 失败
                        }
                    }
                }
                    break;
                case 3: { // 处理 '\x??' 前一位十六进制数字
                    auto esc = hex2dec(str[i]);
                    if (esc != -1) {
                        c = (char) esc;
                        status = 4;
                        i++;
                    } else {
                        status = 0; // 失败
                    }
                }
                    break;
                case 4: { // 处理 '\x??' 后一位十六进制数字
                    auto esc = hex2dec(str[i]);
                    if (esc != -1) {
                        c *= 10;
                        c += (char) esc;
                        ss << c;
                        status = 1;
                        i++;
                    } else {
                        ss << c;
                        status = 1;
                    }
                }
                    break;
                default: // 失败
                    bags._string = str.substr(index + 1, j - index - 1);
                    move(j - index + 1);
                    return l_string;
            }
        }
        if (status == 1) { // 为初态/终态
            bags._string = ss.str();
            move(j - index + 1);
            return l_string;
        }
        bags._string = str.substr(index + 1, j - index - 1);
        move(j - index + 1);
        return l_string;
    }

    lexer_t clexer::next_comment() {
        if (std::regex_search(str.cbegin() + index, str.cend(), sm, r_comment)) {
            auto ms = sm[0].str();
            auto ml = ms.length();
            if (sm[1].matched) { // comment like '// ...'
                bags._comment = sm[1].str();
                move(ml);
                return l_comment;
            }
            if (sm[2].matched) { // comment like '/* ... */'
                bags._comment = sm[2].str();
                move(ml, std::count(bags._comment.begin(), bags._comment.end(), '\n'), true); // check new line
                return l_comment;
            }
        }
        move(length - index); // move to end
        return record_error(e_invalid_comment, length - index);
    }

    lexer_t clexer::next_operator() {
        for (auto i = 2; i >= 0; i--) {
            if (index + i >= length)
                continue;
            if (std::regex_search(str.cbegin() + index, str.cbegin() + index + i + 1, sm, r_operator[i])) {
                auto s = sm[0].str();
                auto b = sm.begin() + 1;
                auto j = std::distance(b, std::find_if(b, sm.end(), match_pred));
                j += lexer_operator_start_idx(i + 1) - 1;
                bags._operator = (operator_t) (j + 1);
                move(s.length());
                return l_operator;
            }
        }
        return record_error(e_invalid_operator, 1);
    }

    int clexer::local() {
        if (index < length)
            return str[index];
        return -1;
    }

    int clexer::local(int offset) {
        if (index + offset < length)
            return str[index + offset];
        return -1;
    }

    void clexer::initMap() {
        // Keyword
        for (auto i = k__start + 1; i < k__end; i++) {
            mapKeyword[KEYWORD_STRING((keyword_t) i)] = (keyword_t) i;
        }
    }
}
