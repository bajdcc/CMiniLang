cmake_minimum_required(VERSION 3.12)
project(CMiniLang)

set(CMAKE_CXX_STANDARD 14)

add_executable(CMiniLang main.cpp types.cpp types.h memory.h clexer.cpp clexer.h cparser.cpp cparser.h cgen.cpp cgen.h cvm.cpp cvm.h)
add_executable(test_lexer test/test_lexer.cpp types.cpp types.h clexer.cpp clexer.h)

enable_testing()
add_test(NAME test_lexer COMMAND test_lexer)
set_tests_properties(test_lexer PROPERTIES PASS_REGULAR_EXPRESSION "pass")