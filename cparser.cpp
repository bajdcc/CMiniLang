//
// Project: CMiniLang
// Author: bajdcc
//
#include <sstream>
#include "cparser.h"
#include "clexer.h"
#include "cast.h"

namespace clib {

    /**
     * 递归下降分析器代码参考自：
     *     https://github.com/lotabout/write-a-C-interpreter
     * 作者文章：
     *     http://lotabout.me/2015/write-a-C-interpreter-0
     */

    cparser::cparser(string_t str)
        : lexer(str) {
    }

    cparser::~cparser() = default;

    ast_node *cparser::parse() {
        // 清空词法分析结果
        lexer.reset();
        // 清空AST
        ast.reset();
        // 语法分析（递归下降）
        program();
        return ast.get_root();
    }

    ast_node *cparser::root() const {
        return ast.get_root();
    }

    void cparser::ast_print(ast_node *node, std::ostream &os) {
        ast.print(node, 0, os);
    }

    void cparser::next() {
        lexer_t token;
        do {
            token = lexer.next();
            if (token == l_error) {
                auto err = lexer.recent_error();
                printf("[%04d:%03d] %-12s - %s\n",
                       err.line,
                       err.column,
                       ERROR_STRING(err.err).c_str(),
                       err.str.c_str());
            }
        } while (token == l_newline || token == l_space || token == l_comment || token == l_error);
#if 0
        if (token != l_end) {
            printf("[%04d:%03d] %-12s - %s\n",
                   lexer.get_last_line(),
                   lexer.get_last_column(),
                   LEX_STRING(lexer.get_type()).c_str(),
                   lexer.current().c_str());
        }
#endif
    }

    void cparser::program() {
        next();
        while (!lexer.is_type(l_end)) {
            global_declaration();
        }
    }

    // 表达式
    ast_node *cparser::expression(operator_t level) {
        // 表达式有多种类型，像 `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
        //
        // 1. unit_unary ::= unit | unit unary_op | unary_op unit
        // 2. expr ::= unit_unary (bin_op unit_unary ...)

        ast_node *node = nullptr;
        // unit_unary()
        if (lexer.is_type(l_end)) { // 结尾
            error("unexpected token EOF of expression");
        }
        if (lexer.is_integer()) { // 数字
            auto tmp = lexer.get_integer();
            match_number();

            node = ast.new_node(ast_int);
            node->data._int = tmp;
        } else if (lexer.is_type(l_string)) { // 字符串
            std::stringstream ss;
            ss << lexer.get_string();
#if 0
            printf("[%04d:%03d] String> %04X '%s'\n", clexer.get_line(), clexer.get_column(), idx, clexer.get_string().c_str());
#endif
            match_type(l_string);

            while (lexer.is_type(l_string)) {
                ss << lexer.get_string();
#if 0
                printf("[%04d:%03d] String> %04X '%s'\n", clexer.get_line(), clexer.get_column(), idx, clexer.get_string().c_str());
#endif
                match_type(l_string);
            }

            node = ast.new_node(ast_string);
            ast.set_str(node, ss.str());
        } else if (lexer.is_keyword(k_sizeof)) { // sizeof
            // 支持 `sizeof(int)`, `sizeof(char)` and `sizeof(*...)`
            match_keyword(k_sizeof);
            match_operator(op_lparan);

            if (lexer.is_keyword(k_unsigned)) {
                match_keyword(k_unsigned); // 有符号或无符号大小相同
            }

            auto size = lexer.get_sizeof();
            next();

            while (lexer.is_operator(op_times)) {
                match_operator(op_times);
                if (size != LEX_SIZEOF(ptr)) {
                    size = LEX_SIZEOF(ptr);
                }
            }

            match_operator(op_rparan);

            node = ast.new_node(ast_int);
            node->data._int = size;
        } else if (lexer.is_type(l_identifier)) { // 变量
            // 三种可能
            // 1. function call 函数名调用
            // 2. Enum variable 枚举值
            // 3. global/local variable 全局/局部变量名
            auto id = lexer.get_identifier();
            match_type(l_identifier);

            if (lexer.is_operator(op_lparan)) { // 函数调用
                // function call
                match_operator(op_lparan);

                node = ast.new_node(ast_invoke);

                // pass in arguments
                while (!lexer.is_operator(op_rparan)) { // 参数数量
                    cast::set_child(node, expression(op_assign));

                    if (lexer.is_operator(op_comma)) {
                        match_operator(op_comma);
                    }
                }

                match_operator(op_rparan);
            } else {
                node = ast.new_node(ast_id);
                ast.set_id(node, id);
            }
        } else if (lexer.is_operator(op_lparan)) { // 强制转换
            // cast or parenthesis
            match_operator(op_lparan);
            if (lexer.is_type(l_keyword)) {
                auto tmp = parse_type();
                auto ptr = 0;

                while (lexer.is_operator(op_times)) {
                    match_operator(op_times);
                    ptr++;
                }
                match_operator(op_rparan);

                node = ast.new_node(ast_cast);
                node->data._type.type = tmp;
                node->data._type.ptr = ptr;
                cast::set_child(node, expression(op_plus_plus));
            } else {
                // 普通括号嵌套
                node = expression(op_assign);
                match_operator(op_rparan);
            }
        } else if (lexer.is_operator(op_times)) { // 解引用
            // dereference *<addr>
            match_operator(op_times);
            node = ast.new_node(ast_sinop);
            node->data._op.op = op_times;
            cast::set_child(node, expression(op_plus_plus));
        } else if (lexer.is_operator(op_bit_and)) { // 取地址
            // get the address of
            match_operator(op_bit_and);
            node = ast.new_node(ast_sinop);
            node->data._op.op = op_bit_and;
            cast::set_child(node, expression(op_plus_plus));
        } else if (lexer.is_operator(op_logical_not)) {
            // not
            match_operator(op_logical_not);
            node = ast.new_node(ast_binop);
            node->data._op.op = op_logical_not;
            cast::set_child(node, expression(op_plus_plus));
        } else if (lexer.is_operator(op_bit_not)) {
            // bitwise not
            match_operator(op_bit_not);
            node = ast.new_node(ast_binop);
            node->data._op.op = op_bit_not;
            cast::set_child(node, expression(op_plus_plus));
        } else if (lexer.is_operator(op_plus)) {
            // +var, do nothing
            match_operator(op_plus);
            node = ast.new_node(ast_binop);
            node->data._op.op = op_plus;
            cast::set_child(node, expression(op_plus_plus));
        } else if (lexer.is_operator(op_minus)) {
            // -var
            match_operator(op_minus);

            if (lexer.is_integer()) {
                node = ast.new_node(ast_int);
                node->data._int = -lexer.get_integer();
                match_integer();
            } else {
                node = ast.new_node(ast_sinop);
                node->data._op.op = op_minus;
                cast::set_child(node, expression(op_plus_plus));
            }
        } else if (lexer.is_operator(op_plus_plus, op_minus_minus)) {
            auto tmp = lexer.get_operator();
            match_type(l_operator);
            node = ast.new_node(ast_binop);
            node->data._op.op = tmp;
            cast::set_child(node, expression(op_plus_plus));
        } else {
            error("bad expression");
        }

        // 二元表达式以及后缀操作符
        operator_t op = op__start;
        while (lexer.is_type(l_operator) && OPERATOR_PRED(op = lexer.get_operator()) <= OPERATOR_PRED(level)) { // 优先级判断
            auto tmp = node;
            switch (op) {
                case op_rparan:
                case op_rsquare:
                case op_colon:
                    return node;
                case op_assign: {
                    // var = expr;
                    match_operator(op_assign);
                    node = ast.new_node(ast_binop);
                    node->data._op.op = op_assign;
                    cast::set_child(node, tmp);
                    cast::set_child(node, expression(op_assign));
                }
                    break;
                case op_query: {
                    // expr ? a : b;
                    match_operator(op_query);
                    node = ast.new_node(ast_triop);
                    node->data._op.op = op_query;
                    cast::set_child(node, tmp);
                    cast::set_child(node, expression(op_assign));

                    if (lexer.is_operator(op_colon)) {
                        match_operator(op_colon);
                    } else {
                        error("missing colon in conditional");
                    }

                    cast::set_child(node, expression(op_query));
                }
                    break;
                case op_plus_plus:
                case op_minus_minus:
                    match_operator(op);
                    node = ast.new_node(ast_sinop);
                    node->data._op.op = op;
                    node->data._op.data = 1;
                    cast::set_child(node, tmp);
                    break;
                case op_equal:
                case op_plus:
                case op_plus_assign:
                case op_minus:
                case op_minus_assign:
                case op_times:
                case op_times_assign:
                case op_divide:
                case op_div_assign:
                case op_bit_and:
                case op_and_assign:
                case op_bit_or:
                case op_or_assign:
                case op_bit_xor:
                case op_xor_assign:
                case op_mod:
                case op_mod_assign:
                case op_less_than:
                case op_less_than_or_equal:
                case op_greater_than:
                case op_greater_than_or_equal:
                case op_not_equal:
                case op_logical_and:
                case op_logical_or:
                case op_pointer:
                case op_left_shift:
                case op_right_shift:
                case op_left_shift_assign:
                case op_right_shift_assign:
                    match_operator(op);
                    node = ast.new_node(ast_binop);
                    node->data._op.op = op;
                    cast::set_child(node, tmp);
                    cast::set_child(node, expression(op));
                    break;
                default:
                    error("compiler error, token = " + lexer.current());
                    break;
            }
        }

        return node;
    }

    // 基本语句
    void cparser::statement() {
        // there are 8 kinds of statements here:
        // 1. if (...) <statement> [else <statement>]
        // 2. while (...) <statement>
        // 3. { <statement> }
        // 4. return xxx;
        // 5. <empty statement>;
        // 6. expression; (expression end with semicolon)

        if (lexer.is_keyword(k_if)) { // if判断
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
            //
            match_keyword(k_if);

            ast.new_child(ast_if);
            match_operator(op_lparan);

            ast.add_child(expression(op_assign)); // if判断的条件

            match_operator(op_rparan);

            ast.new_child(ast_stmt);
            statement();  // 处理if执行体
            ast.to(to_parent);

            if (lexer.is_keyword(k_else)) { // 处理else
                match_keyword(k_else);

                ast.new_child(ast_stmt);
                statement();
                ast.to(to_parent);
            }
            ast.to(to_parent);
        } else if (lexer.is_keyword(k_while)) { // while循环
            //
            // a:                     a:
            //    while (<cond>)        <cond>
            //                          JZ b
            //     <statement>          <statement>
            //                          JMP a
            // b:                     b:
            match_keyword(k_while);

            ast.new_child(ast_while);
            match_operator(op_lparan);

            ast.add_child(expression(op_assign)); // 条件

            match_operator(op_rparan);

            ast.new_child(ast_stmt);
            statement();
            ast.to(to_parent);

        } else if (lexer.is_operator(op_lbrace)) { // 语句
            // { <statement> ... }
            match_operator(op_lbrace);

            ast.new_child(ast_block);
            while (!lexer.is_operator(op_rbrace)) {
                ast.new_child(ast_stmt);
                statement();
                ast.to(to_parent);
            }
            ast.to(to_parent);

            match_operator(op_rbrace);
        } else if (lexer.is_keyword(k_return)) { // 返回
            // return [expression];
            match_keyword(k_return);

            ast.new_child(ast_block);
            if (!lexer.is_operator(op_semi)) {
                ast.add_child(expression(op_assign));
            }
            ast.to(to_parent);

            match_operator(op_semi);
        } else if (lexer.is_operator(op_semi)) { // 空语句
            // empty statement
            match_operator(op_semi);
        } else { // 表达式
            // a = b; or function_call();
            ast.new_child(ast_exp);
            ast.add_child(expression(op_assign));
            ast.to(to_parent);
            match_operator(op_semi);
        }
    }

    // 枚举声明
    void cparser::enum_declaration() {
        // parse enum [id] { a = 1, b = 3, ...}
        int i = 0;
        while (!lexer.is_operator(op_rbrace)) {
            if (!lexer.is_type(l_identifier)) {
                error("bad enum identifier " + lexer.current());
            }
            match_type(l_identifier);
            if (lexer.is_operator(op_assign)) { // 赋值
                // like { a = 10 }
                next();
                if (!lexer.is_integer()) {
                    error("bad enum initializer");
                }
                i = lexer.get_integer();
                next();
            }

            // 保存值到变量中
            ast.new_child(ast_enum_unit);
            auto _id = ast.new_child(ast_id, false);
            auto _int = ast.new_child(ast_int, false);

            ast.set_id(_id, lexer.get_identifier());
            _int->data._int = i++;
            ast.to(to_parent);

            if (lexer.is_operator(op_comma)) {
                next();
            }
        }
    }

    // 函数参数
    void cparser::function_parameter() {
        auto params = 0;
        while (!lexer.is_operator(op_rparan)) { // 判断参数右括号结尾
            // int name, ...
            auto type = parse_type(); // 基本类型
            auto ptr = 0;

            // pointer type
            while (lexer.is_operator(op_times)) { // 指针
                match_operator(op_times);
                ptr++;
            }

            // parameter name
            if (!lexer.is_type(l_identifier)) {
                error("bad parameter declaration");
            }

            auto id = lexer.get_identifier();
            match_type(l_identifier);

            ast.new_child(ast_var_param);
            auto _type = ast.new_child(ast_type, false);
            auto _id = ast.new_child(ast_id, false);
            ast.to(to_parent);

            // 保存本地变量
            _type->data._type.type = type;
            _type->data._type.ptr = ptr;
            ast.set_id(_id, id);

            if (lexer.is_operator(op_comma)) {
                match_operator(op_comma);
            }
        }
    }

    // 函数体
    void cparser::function_body() {
        // type func_name (...) {...}
        //                   -->|   |<--

        { // ...
            // 1. local declarations
            // 2. statements
            // }

            while (lexer.is_basetype()) {
                // 处理基本类型
                base_type = parse_type();

                while (!lexer.is_operator(op_semi)) { // 判断语句结束
                    auto ptr = 0;
                    auto type = base_type;
                    while (lexer.is_operator(op_times)) { // 处理指针
                        match_operator(op_times);
                        ptr++;
                    }

                    if (!lexer.is_type(l_identifier)) {
                        // invalid declaration
                        error("bad local declaration");
                    }

                    auto id = lexer.get_identifier();
                    match_type(l_identifier);

                    ast.new_child(ast_var_local);
                    auto _type = ast.new_child(ast_type, false);
                    auto _id = ast.new_child(ast_id, false);
                    ast.to(to_parent);

                    // 保存本地变量
                    _type->data._type.type = type;
                    _type->data._type.ptr = ptr;
                    ast.set_id(_id, id);

                    if (lexer.is_operator(op_comma)) {
                        match_operator(op_comma);
                    }
                }
                match_operator(op_semi);
            }

            ast.new_child(ast_stmt);
            // statements
            while (!lexer.is_operator(op_rbrace)) {
                statement();
            }
            ast.to(to_parent);
        }
    }

    // 函数声明
    void cparser::function_declaration() {
        // type func_name (...) {...}
        //               | this part

        match_operator(op_lparan);
        ast.new_child(ast_param);
        function_parameter();
        ast.to(to_parent);
        match_operator(op_rparan);
        match_operator(op_lbrace);
        ast.new_child(ast_block);
        function_body();
        ast.to(to_parent);
        // match('}'); 这里不处理右括号是为了上层函数判断结尾
    }

    // 变量声明语句(全局或函数体内)
    // int [*]id [; | (...) {...}]
    void cparser::global_declaration() {
        base_type = l_int;

        // 处理enum枚举
        if (lexer.is_keyword(k_enum)) {
            // enum [id] { a = 10, b = 20, ... }
            match_keyword(k_enum);
            if (!lexer.is_operator(op_lbrace)) {
                match_type(l_identifier); // 省略了[id]枚举名
            }
            ast.new_child(ast_enum);
            if (lexer.is_operator(op_lbrace)) {
                // 处理枚举体
                match_operator(op_lbrace);
                enum_declaration(); // 枚举的变量声明部分，即 a = 10, b = 20, ...
                match_operator(op_rbrace);
            }
            ast.to(to_parent);

            match_operator(op_semi);
            return;
        }

        // 解析基本类型，即变量声明时的类型
        base_type = parse_type();
        if (base_type == l_none)
            base_type = l_int;

        // 处理逗号分隔的变量声明
        while (!lexer.is_operator(op_semi, op_rbrace)) {
            auto type = base_type; // 以先声明的类型为基础
            auto ptr = 0;
            // 处理指针, 像`int ****x;`
            while (lexer.is_operator(op_times)) {
                match_operator(op_times);
                ptr++;
            }

            if (!lexer.is_type(l_identifier)) { // 不存在变量名则报错
                // invalid declaration
                error("bad global declaration");
            }
            auto id = lexer.get_identifier();
            match_type(l_identifier);

            if (lexer.is_operator(op_lparan)) { // 有左括号则应判定是函数声明
                ast.new_child(ast_func);
                auto _type = ast.new_child(ast_type, false);
                auto _id = ast.new_child(ast_id, false);

                // 处理变量声明
                _type->data._type.type = type;
                _type->data._type.ptr = ptr;
                ast.set_id(_id, id);
#if 0
                printf("[%04d:%03d] Function> %04X '%s'\n", clexer.get_line(), clexer.get_column(), id->value._int * 4, id->name.c_str());
#endif
                function_declaration();
                ast.to(to_parent);
            } else {
                ast.new_child(ast_var_global);
                auto _type = ast.new_child(ast_type, false);
                auto _id = ast.new_child(ast_id, false);

                // 处理变量声明
                _type->data._type.type = type;
                _type->data._type.ptr = ptr;
                ast.set_id(_id, id);
                ast.to(to_parent);
#if 0
                printf("[%04d:%03d] Global> %04X '%s'\n", clexer.get_line(), clexer.get_column(), id->value._int, id->name.c_str());
#endif
            }

            if (lexer.is_operator(op_comma)) {
                match_operator(op_comma);
            }
        }
        next();
    }

    void cparser::expect(bool flag, const string_t &info) {
        if (!flag) {
            error(info);
        }
    }

    void cparser::match_keyword(keyword_t type) {
        expect(lexer.is_keyword(type), "expect keyword");
        next();
    }

    void cparser::match_operator(operator_t type) {
        expect(lexer.is_operator(type), "expect operator");
        next();
    }

    void cparser::match_type(lexer_t type) {
        expect(lexer.is_type(type), "expect type");
        next();
    }

    void cparser::match_number() {
        expect(lexer.is_number(), "expect number");
        next();
    }

    void cparser::match_integer() {
        expect(lexer.is_integer(), "expect integer");
        next();
    }

    // 处理基本类型
    // 分char,short,int,long以及相应的无符号类型(无符号暂时不支持)
    // 以及float和double(暂时不支持)
    lexer_t cparser::parse_type() {
        auto type = l_int;
        if (lexer.is_type(l_keyword)) {
            auto _unsigned = false;
            if (lexer.is_keyword(k_unsigned)) { // 判定是否带有unsigned前缀
                _unsigned = true;
                match_keyword(k_unsigned);
            }
            type = lexer.get_typeof(_unsigned); // 根据keyword得到lexer_t
            match_type(l_keyword);
        }
        return type;
    }

    void cparser::error(const string_t &info) {
        printf("[%04d:%03d] ERROR: %s\n", lexer.get_line(), lexer.get_column(), info.c_str());
        throw std::exception();
    }
}
