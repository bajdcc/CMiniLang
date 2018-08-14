//
// Project: CMiniLang
// Author: bajdcc
//
#ifndef CMINILANG_LEXER_H
#define CMINILANG_LEXER_H

#include <bitset>
#include <vector>
#include "types.h"

namespace clib {

    // 词法分析
    class clexer {
    public:
        explicit clexer(string_t str);
        ~clexer();

        // 外部接口
#define DEFINE_LEXER_GETTER(t) LEX_T(t) get_##t() const;
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
#define DEFINE_LEXER_GETTER(t) LEX_T(t) get_store_##t(int) const;
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

    public:
        struct err_record_t {
            int line, column;
            uint start_idx, end_idx;
            error_t err;
            string_t str;
        };

    private:
        std::vector<err_record_t> records;

        lexer_t record_error(error_t error, int skip);

    public:
        lexer_t next();

        lexer_t get_type() const;
        int get_line() const;
        int get_column() const;
        int get_last_line() const;
        int get_last_column() const;
        string_t current() const;

        const err_record_t& recent_error() const;
        std::string store_start();

        lexer_t digit_type(lexer_t t, int i);
        bool digit_from_integer(lexer_t t, LEX_T(ulong) n);
        bool digit_from_double(lexer_t t, LEX_T(double) n);
        lexer_t digit_return(lexer_t t, LEX_T(ulong) n, LEX_T(double) d, int i);

    private:
        void move(int idx, int inc = -1);

        // 内部解析
        lexer_t next_digit();
        lexer_t next_alpha();
        lexer_t next_space();
        lexer_t next_char();
        lexer_t next_string();
        lexer_t next_comment();
        lexer_t next_operator();

        int local();
        int local(int offset);

        string_t store();

    public:
        bool is_type(lexer_t) const;
        bool is_keyword(keyword_t) const;
        bool is_operator(operator_t) const;
        bool is_operator(operator_t, operator_t) const;
        bool is_number() const;
        bool is_integer() const;
        bool is_basetype() const;

        LEX_T(uint) get_integer() const;
        LEX_T(int) get_sizeof() const;
        lexer_t get_typeof(bool) const;

        void reset();

    private:
        string_t str;
        uint index{0};
        uint last_index{0};
        int length{0};

        lexer_t type{l_none};
        int line{1};
        int column{1};
        int last_line{1};
        int last_column{1};

        struct {
#define DEFINE_LEXER_GETTER(t) LEX_T(t) _##t;
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
        } bags;

        struct {
#define DEFINE_LEXER_STORAGE(t) std::vector<LEX_T(t)> _##t;
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
            DEFINE_LEXER_STORAGE(operator)
            DEFINE_LEXER_STORAGE(keyword)
            DEFINE_LEXER_STORAGE(identifier)
            DEFINE_LEXER_STORAGE(string)
            DEFINE_LEXER_STORAGE(comment)
            DEFINE_LEXER_STORAGE(space)
            DEFINE_LEXER_STORAGE(newline)
            DEFINE_LEXER_STORAGE(error)
#undef DEFINE_LEXER_STORAGE
        } storage;

        // 字典
        map_t<string_t, keyword_t> mapKeyword;
        std::bitset<128> bitOp[2];
        std::array<operator_t, 0x100> sinOp;

        void initMap();
    };
}

#endif //CMINILANG_LEXER_H