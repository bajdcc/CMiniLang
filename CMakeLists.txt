cmake_minimum_required(VERSION 3.12)
project(CMiniLang)

set(CMAKE_CXX_STANDARD 14)

add_executable(CMiniLang main.cpp main.cpp types.cpp types.h memory.h clexer.cpp clexer.h cparser.cpp cparser.h cgen.cpp cgen.h cvm.cpp cvm.h)