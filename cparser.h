//
// Project: CMiniLang
// Author: bajdcc
//
#ifndef CMINILANG_PARSER_H
#define CMINILANG_PARSER_H

#include "types.h"
#include "clexer.h"
#include "cast.h"
#include "cgen.h"

namespace clib {

    class cparser {
    public:
        explicit cparser(string_t str);
        ~cparser();

        ast_node *parse();
        ast_node *root() const;
        void ast_print(ast_node *node, std::ostream &os);

    private:
        void next();

        void program();
        ast_node *expression(operator_t level);
        ast_node *statement();
        void enum_declaration();
        void function_parameter();
        void function_body();
        void function_declaration();
        void global_declaration();

    private:
        void expect(bool, const string_t &);
        void match_keyword(keyword_t);
        void match_operator(operator_t);
        void match_type(lexer_t);
        void match_number();
        void match_integer();

        lexer_t parse_type();

        void error(const string_t &);

    private:
        lexer_t base_type{l_none};

    private:
        clexer lexer;
        cast ast;
    };
}
#endif //CMINILANG_PARSER_H