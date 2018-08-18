//
// Project: CMiniLang
// Author: bajdcc
//

#include <cstdio>
#include <fstream>
#include <iostream>
#include "cparser.h"
#include "cvm.h"

extern int g_argc;
extern char** g_argv;

int main(int argc, char **argv)
{
    g_argc = argc;
    g_argv = argv;
#if 1
    string_t txt = R"(
int fibonacci(int i) {
    if (i <= 1)
        return 1;
    return fibonacci(i - 1) + fibonacci(i - 2);
}

void move(char x, char y)
{
    printf("%c --> %c\n", x, y);
}

void hanoi(int n, char a, char b, char c)
{
    if (n == 1)
    {
        move(a, c);
    }
    else
    {
        hanoi(n-1, a, c, b);
        move(a, c);
        hanoi(n-1, b, a, c);
    }
}

int main()
{
    int i;
    i = 0;
    printf("##### fibonacci #####\n");
    while (i <= 10) {
        printf("fibonacci(%2d) = %d\n", i, fibonacci(i));
        i++;
    }
    printf("##### hanoi #####\n");
    hanoi(3, 'A', 'B', 'C');
    return 0;
})";
    try {
        clib::cparser p(txt);
        auto root = p.parse();
        clib::cast::print(root, 0, std::cout);
        clib::cgen gen(root);
        gen.eval();
    } catch (const std::exception& e) {
        printf("ERROR: %s\n", e.what());
    }
#else
    g_argc--;
    g_argv++;
    if (g_argc < 1) {
        printf("Usage: CMiniLang file ...\n");
        return -1;
    }
    std::ifstream in(*g_argv);
    std::istreambuf_iterator<char> beg(in), end;
    std::string str(beg, end);
    if (str.empty()) {
        exit(-1);
    }
    try {
        clib::cparser p(str);
        auto root = p.parse();
        //clib::cast::print(root, 0, std::cout);
        clib::cgen gen(root);
        gen.eval();
    } catch (const std::exception& e) {
        printf("ERROR: %s\n", e.what());
    }
#endif
    return 0;
}
