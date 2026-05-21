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

void test_nontype_double(S_nontype_double<1.5> x);
void test_nontype_double_inf(S_nontype_double<CONST_INF> x);
void test_nontype_double_nan(S_nontype_double<CONST_NAN> x);
void test_nontype_double_neginf(S_nontype_double<CONST_NEGINF> x);

int main() {
  printf("[Consumer20] Testing isolated non-type double template argument interop...\n");

  S_nontype_double<1.5> s;
  test_nontype_double(s);

  S_nontype_double<CONST_INF> s_inf;
  test_nontype_double_inf(s_inf);

#ifdef __clang__
  S_nontype_double<CONST_NAN> s_nan;
  test_nontype_double_nan(s_nan);
#else
  printf("[Consumer20] GCC 2.95 consumer skipping NaN test due to C++98 NaN template incompatibility.\n");
#endif

  S_nontype_double<CONST_NEGINF> s_neginf;
  test_nontype_double_neginf(s_neginf);

  printf("[Consumer20] Isolated interop passed!\n");
  return 0;
}
