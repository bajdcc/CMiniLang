//
// Project: CMiniLang
// Author: bajdcc
//

#include <cstdio>
#include <cassert>
#include "../cparser.h"

using namespace clib;

#define TEST_DOUBLE

#define DEFINE_TEST(t, f) \
void test_##t(const char *s, const LEX_T(t) &v) { \
    printf("[TEST] %-10s: %-20s ", #t, s); \
    clexer lexer(s); \
    if (lexer.next() != LEX_CONV_T(LEX_T(t))) { \
        printf("\nERROR TYPE! ACTUAL: %s REQUIRED: %s", LEX_STRING(lexer.get_type()).c_str(), LEX_STRING(LEX_CONV_T(LEX_T(t))).c_str()); \
        exit(-1); \
    } \
    if (lexer.get_##t() != v) { \
        printf("\nERROR VALUE! ACTUAL: %s REQUIRED: ", lexer.current().c_str()); \
        printf(f, v); \
        exit(-1); \
    } \
    printf(" PASS\n"); \
}

DEFINE_TEST(int, "%d")
DEFINE_TEST(uint, "%ud")
DEFINE_TEST(long, "%ld")
DEFINE_TEST(double, "%lf")

int main(int argc, char **argv) {
#ifdef TEST_DOUBLE
    test_int("123", 123);
    test_int("123.", 123);
    test_double("123.0", 123.0);
    test_int("123.0I", 123);
    test_uint("123.0UI", 123U);
    test_double("123.4", 123.4);
    test_double("123e1", 123e1);
    test_double("123.e1", 123.e1);
    test_double("123.4e1", 123.4e1);
    test_double("123e-1", 123e-1);
    test_double("123.e-1", 123.e-1);
    test_double("123.4e-1", 123.4e-1);
    test_long("12345678987654321", 12345678987654321LL);
    test_double("1e20", 1e20);
#endif
    printf("ALL PASS");
    return 0;
}
