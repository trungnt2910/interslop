#include <stdio.h>

#ifdef __clang__
#define CONST_INF __builtin_inf()
#define CONST_NAN __builtin_nan("")
#define CONST_NEGINF -__builtin_inf()
#else
#define CONST_INF (1.0/0.0)
#define CONST_NAN (0.0/0.0)
#define CONST_NEGINF (-1.0/0.0)
#endif

template <double D> struct S_nontype_double {};

void test_nontype_double(S_nontype_double<1.5> x) {
  printf("[Lib20] test_nontype_double (1.5) passed\n");
}

void test_nontype_double_inf(S_nontype_double<CONST_INF> x) {
  printf("[Lib20] test_nontype_double_inf passed\n");
}

void test_nontype_double_nan(S_nontype_double<CONST_NAN> x) {
  printf("[Lib20] test_nontype_double_nan passed\n");
}

void test_nontype_double_neginf(S_nontype_double<CONST_NEGINF> x) {
  printf("[Lib20] test_nontype_double_neginf passed\n");
}
