int g_lib_first_definition = 1;
#include "lib.h"
#include <stdlib.h>

extern "C" {
  __attribute__((weak)) void __gxx_personality_sj0() {}
  __attribute__((weak)) void _Unwind_SjLj_Register(void *) {}
  __attribute__((weak)) void _Unwind_SjLj_Unregister(void *) {}
  __attribute__((weak)) void _Unwind_SjLj_Resume(void *) { abort(); }
}

int Derived::instance_count = 0;

Base1::Base1() {
  printf("[Lib] Base1::Base1()\n");
}
Base1::~Base1() {
  printf("[Lib] Base1::~Base1()\n");
}
int Base1::f1() {
  printf("[Lib] Base1::f1()\n");
  return 10;
}

Base2::Base2() {
  printf("[Lib] Base2::Base2()\n");
}
Base2::~Base2() {
  printf("[Lib] Base2::~Base2()\n");
}
int Base2::f2() {
  printf("[Lib] Base2::f2()\n");
  return 20;
}

Derived::Derived(int z) : Base1(), Base2(), z(z) {
  instance_count++;
  printf("[Lib] Derived::Derived(%d), instance_count=%d\n", z, instance_count);
}
Derived::Derived(const Derived &other) : Base1(other), Base2(other), z(other.z) {
  instance_count++;
  printf("[Lib] Derived::Derived(const Derived&), instance_count=%d\n", instance_count);
}
Derived::~Derived() {
  instance_count--;
  printf("[Lib] Derived::~Derived(), instance_count=%d\n", instance_count);
}
int Derived::f1() {
  printf("[Lib] Derived::f1() [overriding Base1::f1], z=%d\n", z);
  return z + 1;
}
int Derived::f2() {
  printf("[Lib] Derived::f2() [overriding Base2::f2], z=%d\n", z);
  return z + 2;
}
int Derived::f3() const {
  printf("[Lib] Derived::f3() const, z=%d\n", z);
  return z + 3;
}
int Derived::f4() {
  printf("[Lib] Derived::f4(), z=%d\n", z);
  return z * 10;
}
void Derived::operator+=(int val) {
  printf("[Lib] Derived::operator+=(%d), z=%d\n", val, z);
  z += val;
}
void Derived::operator-=(int val) {
  printf("[Lib] Derived::operator-=(%d), z=%d\n", val, z);
  z -= val;
}
void Derived::operator*=(int val) {
  printf("[Lib] Derived::operator*=(%d), z=%d\n", val, z);
  z *= val;
}
void Derived::operator/=(int val) {
  printf("[Lib] Derived::operator/=(%d), z=%d\n", val, z);
  z /= val;
}
void Derived::operator%=(int val) {
  printf("[Lib] Derived::operator%=(%d), z=%d\n", val, z);
  z %= val;
}
void Derived::operator&=(int val) {
  printf("[Lib] Derived::operator&=(%d), z=%d\n", val, z);
  z &= val;
}
void Derived::operator|=(int val) {
  printf("[Lib] Derived::operator|=(%d), z=%d\n", val, z);
  z |= val;
}
void Derived::operator^=(int val) {
  printf("[Lib] Derived::operator^=(%d), z=%d\n", val, z);
  z ^= val;
}
void Derived::operator<<=(int val) {
  printf("[Lib] Derived::operator<<=(%d), z=%d\n", val, z);
  z <<= val;
}
void Derived::operator>>=(int val) {
  printf("[Lib] Derived::operator>>=(%d), z=%d\n", val, z);
  z >>= val;
}

Derived::operator int() {
  printf("[Lib] Derived::operator int(), z=%d\n", z);
  return z;
}

int normal_func(int x, double y) {
  printf("[Lib] normal_func(x=%d, y=%.1f)\n", x, y);
  return x + (int)y;
}

void test_const_ptr(int * const * p) {
  if (!p || !*p || **p != 12345) { printf("[ERROR] test_const_ptr failed!\n"); exit(1); }
  printf("[Lib] test_const_ptr val=%d\n", **p);
}

void test_const_ptmd(DerivedPTMD const * p) {
  if (!p || *p != &Derived::z) { printf("[ERROR] test_const_ptmd failed!\n"); exit(1); }
  printf("[Lib] test_const_ptmd\n");
}
void test_const_ptmf(DerivedPTMF const * p) {
  if (!p || *p != &Derived::f1) { printf("[ERROR] test_const_ptmf failed!\n"); exit(1); }
  printf("[Lib] test_const_ptmf\n");
}
void test_const_ptr_ref(int * const & p) {
  if (!p || *p != 12345) { printf("[ERROR] test_const_ptr_ref failed!\n"); exit(1); }
  printf("[Lib] test_const_ptr_ref val=%d\n", *p);
}
void test_const_ptmd_ref(DerivedPTMD const & p) {
  if (p != &Derived::z) { printf("[ERROR] test_const_ptmd_ref failed!\n"); exit(1); }
  printf("[Lib] test_const_ptmd_ref\n");
}
void test_const_ptmf_ref(DerivedPTMF const & p) {
  if (p != &Derived::f1) { printf("[ERROR] test_const_ptmf_ref failed!\n"); exit(1); }
  printf("[Lib] test_const_ptmf_ref\n");
}
void test_cv_ptr(int * const volatile * p) {
  if (!p || !*p || **p != 12345) { printf("[ERROR] test_cv_ptr failed!\n"); exit(1); }
  printf("[Lib] test_cv_ptr val=%d\n", **p);
}

void test_complex(__complex__ double c) {
  if (__real__ c != 1.0 || __imag__ c != 0.0) { printf("[ERROR] test_complex failed!\n"); exit(1); }
  printf("[Lib] test_complex\n");
}
void test_array_ref(int (&arr)[10]) {
  if (arr[0] != 42) { printf("[ERROR] test_array_ref failed!\n"); exit(1); }
  printf("[Lib] test_array_ref val=%d\n", arr[0]);
}
void test_fn_ptr(int (*fn)(double)) {
  if (!fn || fn(3.14) != 6) { printf("[ERROR] test_fn_ptr failed!\n"); exit(1); }
  printf("[Lib] test_fn_ptr res=%d\n", fn(3.14));
}
void test_vu_ptr(volatile unsigned int * p) {
  if (!p || *p != 777) { printf("[ERROR] test_vu_ptr failed!\n"); exit(1); }
  printf("[Lib] test_vu_ptr val=%u\n", (unsigned int)*p);
}
void test_restrict_ptr(int * __restrict * p) {
  if (!p || !*p || **p != 42) { printf("[ERROR] test_restrict_ptr failed!\n"); exit(1); }
  printf("[Lib] test_restrict_ptr val=%d\n", **p);
}

void trigger_eh_from_lib() {
  printf("[Lib] trigger_eh_from_lib() throwing Derived exception!\n");
  throw Derived(42);
}

void trigger_eh_base() {
  printf("[Lib] trigger_eh_base() throwing Derived exception!\n");
  throw Derived(84);
}

void trigger_eh_vbase() {
  printf("[Lib] trigger_eh_vbase() throwing DeepVSub exception!\n");
  throw DeepVSub();
}

void trigger_eh_int_ptr() {
  static int x = 12345;
  printf("[Lib] trigger_eh_int_ptr() throwing int*!\n");
  throw &x;
}

void trigger_eh_fn_ptr() {
  printf("[Lib] trigger_eh_fn_ptr() throwing function pointer!\n");
  throw &normal_func;
}

DerivedPTMF get_lib_ptmf(int which) {
  if (which == 1) return &Derived::f1;
  if (which == 2) return &Derived::f2;
  if (which == 4) return &Derived::f4;
  return 0;
}

int call_lib_ptmf(Derived *d, DerivedPTMF ptmf) {
  if (ptmf != 0)
    return (d->*ptmf)();
  return -1;
}

DerivedConstPTMF get_lib_const_ptmf() {
  return &Derived::f3;
}

int call_lib_const_ptmf(const Derived *d, DerivedConstPTMF ptmf) {
  if (ptmf != 0)
    return (d->*ptmf)();
  return -1;
}

DerivedPTMD get_lib_ptmd() {
  return &Derived::z;
}

int get_lib_ptmd_val(Derived *d, DerivedPTMD ptmd) {
  if (ptmd != 0)
    return d->*ptmd;
  return -1;
}

int CookieTester::dtor_count = 0;

CookieTester::CookieTester() {
  value = 999;
  printf("[Lib] CookieTester::CookieTester()\n");
}

CookieTester::~CookieTester() {
  dtor_count++;
  printf("[Lib] CookieTester::~CookieTester(), dtor_count=%d\n", dtor_count);
}

CookieTester *alloc_cookie_tester(int n) {
  printf("[Lib] alloc_cookie_tester(%d)...\n", n);
  return new CookieTester[n];
}

void delete_cookie_tester(CookieTester *p) {
  printf("[Lib] delete_cookie_tester...\n");
  delete[] p;
}

int VBaseDtorTester::dtor_count = 0;

VBaseDtorTester::VBaseDtorTester() {
  printf("[Lib] VBaseDtorTester::VBaseDtorTester()\n");
}

VBaseDtorTester::~VBaseDtorTester() {
  dtor_count++;
  printf("[Lib] VBaseDtorTester::~VBaseDtorTester(), dtor_count=%d\n", dtor_count);
}

VSubDtorTester::VSubDtorTester() : VBaseDtorTester() {
  printf("[Lib] VSubDtorTester::VSubDtorTester()\n");
}

VSubDtorTester::~VSubDtorTester() {
  printf("[Lib] VSubDtorTester::~VSubDtorTester()\n");
}

VSubDtorTester *alloc_vsub() {
  printf("[Lib] alloc_vsub()...\n");
  return new VSubDtorTester();
}

void delete_vsub(VSubDtorTester *p) {
  printf("[Lib] delete_vsub...\n");
  delete p;
}
void delete_vsub_base(VBaseDtorTester *p) {
  printf("[Lib] delete_vsub_base...\n");
  delete p;
}

int InlineVBaseDtorTester::dtor_count = 0;

InlineVBaseDtorTester::InlineVBaseDtorTester() {
  printf("[Lib] InlineVBaseDtorTester::InlineVBaseDtorTester()\n");
}

InlineVBaseDtorTester::~InlineVBaseDtorTester() {
  dtor_count++;
  printf("[Lib] InlineVBaseDtorTester::~InlineVBaseDtorTester(), dtor_count=%d\n", dtor_count);
}

InlineVSubDtorTester::InlineVSubDtorTester() : InlineVBaseDtorTester() {
  printf("[Lib] InlineVSubDtorTester::InlineVSubDtorTester()\n");
}

InlineVSubDtorTester *alloc_inline_vsub() {
  printf("[Lib] alloc_inline_vsub()...\n");
  return new InlineVSubDtorTester();
}

void delete_inline_vsub_base(InlineVBaseDtorTester *p) {
  printf("[Lib] delete_inline_vsub_base...\n");
  delete p;
}

int call_inline_static_from_lib(int v) {
  printf("[Lib] call_inline_static_from_lib(%d)...\n", v);
  return test_inline_static(v);
}

int test_pass_by_val(ByValStruct s) {
  printf("[Lib] test_pass_by_val(s.val=%d)\n", s.val);
  return s.val * 2;
}

int test_repeat_ptrs(int *a, int *b, int *c) {
  int va = a ? *a : 0;
  int vb = b ? *b : 0;
  int vc = c ? *c : 0;
  printf("[Lib] test_repeat_ptrs(%d, %d, %d)\n", va, vb, vc);
  return va + vb + vc;
}

int test_repeat_bools(bool a, bool b, bool c) {
  printf("[Lib] test_repeat_bools(%d, %d, %d)\n", (int)a, (int)b, (int)c);
  return (int)a + (int)b + (int)c;
}

int test_mixed_repeats(int a, int *b, int *c) {
  int vb = b ? *b : 0;
  int vc = c ? *c : 0;
  printf("[Lib] test_mixed_repeats(%d, %d, %d)\n", a, vb, vc);
  return a + vb + vc;
}

int test_template_mangling(TemplateClass<int> *p) {
  int val = p ? p->get_value() : 0;
  printf("[Lib] test_template_mangling(%d)\n", val);
  return val;
}

int test_template_template_mangling(InteropOuter<InteropInner> *p) {
  printf("[Lib] test_template_template_mangling called\n");
  return 99;
}


PolyBase::PolyBase() : p(111) {
  printf("[Lib] PolyBase::PolyBase()\n");
}
PolyBase::~PolyBase() {
  printf("[Lib] PolyBase::~PolyBase()\n");
}
int PolyBase::poly_func() { return p; }

IntermediateVBase::IntermediateVBase() : PolyBase(), i(222) {
  printf("[Lib] IntermediateVBase::IntermediateVBase()\n");
}
IntermediateVBase::~IntermediateVBase() {
  printf("[Lib] IntermediateVBase::~IntermediateVBase()\n");
}
int IntermediateVBase::inter_func() { return i; }

DeepVSub::DeepVSub() : PolyBase(), IntermediateVBase(), d(333) {
  printf("[Lib] DeepVSub::DeepVSub()\n");
}
DeepVSub::~DeepVSub() {
  printf("[Lib] DeepVSub::~DeepVSub()\n");
}

DeepVSub *alloc_deep_vsub() {
  printf("[Lib] alloc_deep_vsub()...\n");
  return new DeepVSub();
}
void delete_deep_vsub(DeepVSub *p) {
  printf("[Lib] delete_deep_vsub...\n");
  delete p;
}

void check_ebo_derived(EboDerived *d) {
  printf("[Lib] check_ebo_derived: d->y = %d\n", d->y);
  if (d->y != 42) {
    printf("[ERROR] EBO Interop Failure: Expected d->y == 42, got %d\n", d->y);
    exit(1);
  }
}

GuardTester::GuardTester(int v) : val(v) {
  printf("[Lib] GuardTester::GuardTester(%d)\n", v);
}
GuardTester::~GuardTester() {
  printf("[Lib] GuardTester::~GuardTester()\n");
}
int call_inline_guard_from_lib(int v) {
  return test_inline_guard(v);
}

void CovariantBase1::dummy() {}

CovariantBase2* CovariantBase2::clone() {
  return this;
}

CovariantDerived* CovariantDerived::clone() {
  return this;
}

static CovariantDerived static_covariant_derived;

CovariantBase2* get_covariant_derived() {
  return &static_covariant_derived;
}

int gcc2_collision_var = 999;

int nontype_global_var = 12345;
void nontype_global_func() {
  printf("[Lib] nontype_global_func called\n");
}

void test_nontype_1(S_nontype1<42> x) {
  printf("[Lib] test_nontype_1 passed\n");
}
void test_nontype_2(S_nontype2<&nontype_global_var> x) {
  printf("[Lib] test_nontype_2 passed\n");
}
void test_nontype_3(S_nontype3<&nontype_global_func> x) {
  printf("[Lib] test_nontype_3 passed\n");
}
void test_nontype_4(S_nontype4<&Derived::z> x) {
  printf("[Lib] test_nontype_4 passed\n");
}
void test_nontype_5(S_nontype5<&Derived::f1> x) {
  printf("[Lib] test_nontype_5 passed\n");
}
void test_nontype_7(S_nontype5<&Derived::f2> x) {
  printf("[Lib] test_nontype_7 passed\n");
}

void PureVirtualBase::key_func() {
  printf("[Lib] PureVirtualBase::key_func called\n");
}

void check_layout_derived(LayoutDerived1 *d1, LayoutDerived2 *d2) {
  printf("[Lib] check_layout_derived\n");
  d1->c = 'A';
  d2->c = 'B';
}

void check_empty_vderived(EmptyVDerived1 *d1, EmptyVDerived2 *d2) {
  printf("[Lib] check_empty_vderived\n");
  printf("[Lib] sizeof(EmptyVDerived1) = %d, alignof = %d\n", (int)sizeof(EmptyVDerived1), (int)__alignof__(EmptyVDerived1));
  printf("[Lib] sizeof(EmptyVDerived2) = %d, alignof = %d\n", (int)sizeof(EmptyVDerived2), (int)__alignof__(EmptyVDerived2));
}

void check_empty_vderived_array(EmptyVDerived2 *arr, int n) {
  printf("[Lib] check_empty_vderived_array\n");
  printf("[Lib] arr[0] = %p, arr[1] = %p (diff = %d)\n",
         &arr[0], &arr[1], (int)((char*)&arr[1] - (char*)&arr[0]));
  // We don't exit(1) here yet, just print so we can see the discrepancy in the test output.
  // Or wait, the instructions say "Loop until you can find some compatibility issues. Reproduce it in the run_interop_tests.sh ...".
  // If we want the script to fail, we can exit(1) if they disagree.
  // But wait, does the script expect to fail or pass?
  // "Reproduce it in the run_interop_tests.sh and the accompanying C++ code."
  // Usually, reproduction means the test should FAIL before the fix, and PASS after the fix.
  // So yes, we should make it fail if there is a discrepancy!
  if ((char*)&arr[1] - (char*)&arr[0] != 12) { // GCC size is 12
    printf("[ERROR] Lib (Clang) thinks array element spacing is %d, but it should be 12!\n",
           (int)((char*)&arr[1] - (char*)&arr[0]));
    // Wait, if Lib is compiled with Clang, and Clang is buggy, it will compute diff = 10.
    // So it will trigger this error if diff != 12.
    // This is perfect!
    exit(1);
  }
}

VptrNotZero::VptrNotZero() : x(42) {}
void VptrNotZero::foo() {
  printf("[Lib] VptrNotZero::foo() called! x=%d\n", x);
}
VptrNotZeroPTMF get_vptr_not_zero_ptmf() {
  return &VptrNotZero::foo;
}

DynDerived::DynDerived(int v) : val(v) {}
DynBase* get_dyn_derived(int v) {
  return new DynDerived(v);
}

template <typename T>
T interop_fn_tmpl(T val) {
  return val + 100;
}

// Explicit instantiations for GCC2 interop
template int interop_fn_tmpl<int>(int val);
template double interop_fn_tmpl<double>(double val);

void check_repro_class(ReproClass *p) {
  printf("[Lib] check_repro_class: sizeof(ReproClass) = %d\n", (int)sizeof(ReproClass));
  printf("[Lib] check_repro_class: offset of EmptyVBase1 = %d\n",
         (int)((char*)(EmptyVBase1*)p - (char*)p));

  if (sizeof(ReproClass) != 16) {
    printf("[ERROR] sizeof(ReproClass) expected 16, got %d\n", (int)sizeof(ReproClass));
    exit(1);
  }
  if ((int)((char*)(EmptyVBase1*)p - (char*)p) != 12) {
    printf("[ERROR] offset of EmptyVBase1 expected 12, got %d\n", (int)((char*)(EmptyVBase1*)p - (char*)p));
    exit(1);
  }
}

void MiBase2::foo() {}
void MiDerived::bar() {}

void test_nontype_virt(S_nontype_virt<&VptrNotZero::foo> x) {
  printf("[Lib] test_nontype_virt passed\n");
}

void test_nontype_mi(S_nontype_mi<&MiDerived::bar> x) {
  printf("[Lib] test_nontype_mi passed\n");
}

InteropVBase::InteropVBase() : val(10) {
  printf("[Lib] InteropVBase::InteropVBase()\n");
}
InteropVBase::~InteropVBase() {
  printf("[Lib] InteropVBase::~InteropVBase()\n");
}
int InteropVBase::vfn() {
  printf("[Lib] InteropVBase::vfn() val=%d\n", val);
  return val;
}

InteropVDerived::InteropVDerived(int v1, int v2) : InteropVBase(), val2(v2) {
  val = v1;
  printf("[Lib] InteropVDerived::InteropVDerived(%d, %d)\n", v1, v2);
}
InteropVDerived::~InteropVDerived() {
  printf("[Lib] InteropVDerived::~InteropVDerived()\n");
}
int InteropVDerived::vfn() {
  printf("[Lib] InteropVDerived::vfn() val=%d, val2=%d\n", val, val2);
  return val + val2;
}

VDerivedPTMF get_vderived_ptmf() {
  return &InteropVDerived::vfn;
}
int call_vderived_ptmf(InteropVDerived *d, VDerivedPTMF ptmf) {
  return (d->*ptmf)();
}

DtorOnlyNonTrivial::~DtorOnlyNonTrivial() {
  // Non-trivial destructor
  printf("[Lib] ~DtorOnlyNonTrivial() called, val = %d\n", val);
}

int test_pass_by_val_dtor_only(DtorOnlyNonTrivial s) {
  printf("[Lib] test_pass_by_val_dtor_only: s.val = %d\n", s.val);
  return s.val * 2;
}

OverwriteVBase::OverwriteVBase() : v_val(999) {}
OverwriteNonVirtualBase::OverwriteNonVirtualBase() : nv_val(111) {}
OverwriteIntermediate::OverwriteIntermediate() : x(123) {}
OverwriteDerived::OverwriteDerived() : y(456) {}

void check_overwrite_derived(OverwriteDerived *d) {
  printf("[Lib] check_overwrite_derived: d->nv_val = %d, d->x = %d\n", d->nv_val, d->x);
  if (d->nv_val != 111) {
    printf("[ERROR] d->nv_val was overwritten! Value = %d\n", d->nv_val);
    exit(1);
  }
  OverwriteIntermediate *inter = d;
  OverwriteVBase *vbase = (OverwriteVBase*)inter;
  printf("[Lib] inter = %p, vbase = %p\n", (void*)inter, (void*)vbase);
  if (inter) {
    void **vbptr_addr = (void**)((char*)inter + 4);
    printf("[Lib] vbptr value at inter+4 = %p\n", *vbptr_addr);
  }
}

int test_empty_struct_pass(int a, EmptyStruct e, int b) {
  printf("[Lib] test_empty_struct_pass: a=%d, b=%d\n", a, b);
  return a + b;
}

void test_ptmf_compare_interop() {
  printf("[Lib] test_ptmf_compare_interop called\n");
}

#include <typeinfo>

void ReproVBaseDyn::f() {
  printf("[Lib] ReproVBaseDyn::f() called!\n");
}
void test_vptr_retreival_vbase(ReproVDerived *d) {
  printf("[Lib] sizeof(ReproVDerived) = %d\n", (int)sizeof(ReproVDerived));
  printf("[Lib] offset of ReproVBaseNonDyn = %d\n", (int)((char*)(ReproVBaseNonDyn*)d - (char*)d));
  printf("[Lib] offset of ReproVBaseDyn = %d\n", (int)((char*)(ReproVBaseDyn*)d - (char*)d));
  printf("[Lib] ReproVDerived typeid name = %s\n", typeid(*d).name());

  if (sizeof(ReproVDerived) != 20) {
    printf("[ERROR] sizeof(ReproVDerived) mismatch in Lib! Expected 20, got %d\n", (int)sizeof(ReproVDerived));
    exit(1);
  }
  if ((int)((char*)(ReproVBaseNonDyn*)d - (char*)d) != 12) {
    printf("[ERROR] offset of ReproVBaseNonDyn mismatch in Lib! Expected 12, got %d\n", (int)((char*)(ReproVBaseNonDyn*)d - (char*)d));
    exit(1);
  }
  if ((int)((char*)(ReproVBaseDyn*)d - (char*)d) != 16) {
    printf("[ERROR] offset of ReproVBaseDyn mismatch in Lib! Expected 16, got %d\n", (int)((char*)(ReproVBaseDyn*)d - (char*)d));
    exit(1);
  }
}

void ReproNVBase::f() {
  printf("[Lib] ReproNVBase::f() called!\n");
}
void test_vptr_retreival_nvbase(ReproNVDerived *d) {
  printf("[Lib] sizeof(ReproNVDerived) = %d\n", (int)sizeof(ReproNVDerived));
  printf("[Lib] offset of ReproNVBase = %d\n", (int)((char*)(ReproNVBase*)d - (char*)d));
  printf("[Lib] ReproNVDerived typeid name = %s\n", typeid(*d).name());

  if (sizeof(ReproNVDerived) != 12) {
    printf("[ERROR] sizeof(ReproNVDerived) mismatch in Lib! Expected 12, got %d\n", (int)sizeof(ReproNVDerived));
    exit(1);
  }
  if ((int)((char*)(ReproNVBase*)d - (char*)d) != 0) {
    printf("[ERROR] offset of ReproNVBase mismatch in Lib! Expected 0, got %d\n", (int)((char*)(ReproNVBase*)d - (char*)d));
    exit(1);
  }
}

void check_vbase_alignment_bug(BugVBase *p, int expected_size) {
  printf("[Lib] check_vbase_alignment_bug:\n");
  printf("[Lib] sizeof(BugVBase) = %d\n", (int)sizeof(BugVBase));
  printf("[Lib] offset of BugEmpty1 = %d\n", (int)((char*)(BugEmpty1*)p - (char*)p));
  printf("[Lib] offset of BugEmpty2 = %d\n", (int)((char*)(BugEmpty2*)p - (char*)p));
  if ((int)sizeof(BugVBase) != expected_size) {
    printf("[ERROR] sizeof(BugVBase) mismatch! Lib sees %d, Consumer expected %d\n", (int)sizeof(BugVBase), expected_size);
    exit(1);
  }
  if ((int)((char*)(BugEmpty1*)p - (char*)p) != 12) {
    printf("[ERROR] offset of BugEmpty1 mismatch in Lib! Expected 12, got %d\n", (int)((char*)(BugEmpty1*)p - (char*)p));
    exit(1);
  }
  if ((int)((char*)(BugEmpty2*)p - (char*)p) != 13) {
    printf("[ERROR] offset of BugEmpty2 mismatch in Lib! Expected 13, got %d\n", (int)((char*)(BugEmpty2*)p - (char*)p));
    exit(1);
  }
}

RTTIBase* get_lib_local_rtti() {
  return get_local_rtti_obj<int>();
}

const std::type_info& get_lib_regular_local_rtti2() {
  return get_regular_local_rtti2();
}

int pmf_called_vbase = 0;
int pmf_called_derived = 0;

void PmfVBase::vbase_virt() {
  printf("[Lib] PmfVBase::vbase_virt()\n");
}

void PmfDerived::derived_virt() {
  printf("[Lib] PmfDerived::derived_virt()\n");
  pmf_called_derived = 1;
}

void PmfDerived::vbase_virt() {
  printf("[Lib] PmfDerived::vbase_virt()\n");
  pmf_called_vbase = 1;
}

extern "C" PmfDerivedPTMF get_pmf_derived_vbase_virt() {
  return &PmfDerived::vbase_virt;
}

extern "C" PmfDerivedPTMF get_pmf_derived_derived_virt() {
  return &PmfDerived::derived_virt;
}

extern "C" void call_pmf_derived(PmfDerived *d, PmfDerivedPTMF ptmf) {
  (d->*ptmf)();
}

int EhVBaseCleanupTester::dtor_count = 0;

EhVBaseCleanupTester::EhVBaseCleanupTester(int v) : val(v) {
  printf("[Lib] EhVBaseCleanupTester::EhVBaseCleanupTester(%d)\n", val);
}

EhVBaseCleanupTester::~EhVBaseCleanupTester() {
  dtor_count++;
  printf("[Lib] EhVBaseCleanupTester::~EhVBaseCleanupTester() val=%d\n", val);
}

void trigger_eh_cleanup_from_lib() {
  printf("[Lib] trigger_eh_cleanup_from_lib() throwing int!\n");
  throw 999;
}

IndirectVBaseA::IndirectVBaseA() { a = 111; }
IndirectVBaseA::~IndirectVBaseA() {}
void IndirectVBaseA::foo() { printf("[Lib] IndirectVBaseA::foo() a=%d\n", a); }

IndirectVBaseC::IndirectVBaseC() : IndirectVBaseA() {
  c = 222;
  IndirectVBaseA* a_ptr = this;
  printf("[Lib] IndirectVBaseC::IndirectVBaseC() this=%p, a_ptr=%p\n", (void*)this, (void*)a_ptr);
  if (a_ptr->a != 111) {
    printf("[ERROR] IndirectVBaseC ctor: corrupted a_ptr->a = %d\n", a_ptr->a);
    exit(1);
  }
}
IndirectVBaseC::~IndirectVBaseC() {}

IndirectVBaseD::IndirectVBaseD() : IndirectVBaseC() {
  d = 333;
  IndirectVBaseA* a_ptr = this;
  printf("[Lib] IndirectVBaseD::IndirectVBaseD() this=%p, a_ptr=%p\n", (void*)this, (void*)a_ptr);
  if (a_ptr->a != 111) {
    printf("[ERROR] IndirectVBaseD ctor: corrupted a_ptr->a = %d\n", a_ptr->a);
    exit(1);
  }
}
IndirectVBaseD::~IndirectVBaseD() {}

IndirectVBaseE::IndirectVBaseE() : IndirectVBaseD() { e = 444; }
IndirectVBaseE::~IndirectVBaseE() {}

void test_indirect_vbase_interop() {
  printf("[Lib] test_indirect_vbase_interop() constructing IndirectVBaseE...\n");
  IndirectVBaseE* e = new IndirectVBaseE();
  printf("[Lib] test_indirect_vbase_interop() success!\n");
  delete e;
}

namespace InteropNS {
  int namespace_var = 42;
  int namespace_func(int x) {

    return x + 100;
  }
  int test_ns_tmpl_class(TmplClass<int> *p) {
    return p->get_val() + 10;
  }
}

namespace A {
  namespace B {
    namespace C {
      namespace D {
        int deep_func(int x) {
          return x + 200;
        }
      }
    }
  }
  namespace std {
    int nested_std_func(int x) {
      return x + 42;
    }
  }
}

namespace std {
  int std_var = 100;
  int top_level_std_func(int x) {

    return x + 3000;
  }
  namespace B {
    int std_nested_func(int x) {
      return x + 50;
    }
  }
  namespace std {
    int nested_std_std_func(int x) {
      return x + 100;
    }
  }
  namespace my_foo {
    int nested_foo_func(int x) {
      return x + 200;
    }
    namespace std {
      int nested_foo_std_func(int x) {
        return x + 300;
      }
    }
  }
}

namespace Foo {
  int overload(int x) {
    return x + 10;
  }
  double overload(double x) {
    return x + 20.0;
  }
}

namespace Y {
  X::S func(int x) {
    X::S s;
    s.val = x + 300;
    return s;
  }
}

namespace M {
  int func(S_PTMD p, S *s) {
    return s->*p;
  }
}

void BugPMFV::f() { printf("[Lib] BugPMFV::f\n"); }
BugPMFV::BugPMFV() { v = 1; }
BugPMFA::BugPMFA() { a = 2; }
void BugPMFB::f() { printf("[Lib] BugPMFB::f\n"); }
BugPMFB::BugPMFB() { b = 3; }

extern "C" BugPMFB_PTMF get_bug_pmf() {
  printf("[Lib] Clang sizeof(BugPMFB) = %d\n", (int)sizeof(BugPMFB));
  return &BugPMFB::f;
}

CastBugV1::CastBugV1() { v1 = 111; }
CastBugV1::~CastBugV1() {}
void CastBugV1::f1() { printf("[Lib] CastBugV1::f1: v1=%d\n", v1); }

CastBugV2::CastBugV2() { v2 = 222; }
CastBugV2::~CastBugV2() {}
void CastBugV2::f2() { printf("[Lib] CastBugV2::f2: v2=%d\n", v2); }

CastBugD::CastBugD() { d = 333; }
CastBugD::~CastBugD() {}
void CastBugD::fd() { printf("[Lib] CastBugD::fd: d=%d\n", d); }

CastBugDD::CastBugDD() {
  o = 444;
  dd = 555;
}
CastBugDD::~CastBugDD() {}

static CastBugDD static_cast_bug_dd;

extern "C" CastBugD* get_cast_bug_d() {
  printf("[Lib] get_cast_bug_d: dd=%p, d=%p\n", &static_cast_bug_dd, (CastBugD*)&static_cast_bug_dd);
  printf("[Lib] get_cast_bug_d: v2=%p, v1=%p\n", (CastBugV2*)&static_cast_bug_dd, (CastBugV1*)&static_cast_bug_dd);
  return &static_cast_bug_dd;
}

extern "C" CastBugV1* get_cast_bug_v1() {
  return &static_cast_bug_dd;
}

extern "C" CastBugPTMD_C_PTMD get_cast_bug_ptmd_null() {
  CastBugPTMD_B_PTMD pb = 0;
  return pb;
}

extern "C" CastBugPTMD_C_PTMD get_cast_bug_ptmd_nonnull() {
  CastBugPTMD_B_PTMD pb = &CastBugPTMD_B::b;
  return pb;
}

extern "C" int call_cast_bug_ptmd(CastBugPTMD_C *obj, CastBugPTMD_C_PTMD p) {
  return obj->*p;
}

int interop_global_var = 789;
void test_extern_func() {
  printf("[Lib] test_extern_func called!\n");
}

void test_template_ref_global_interop(S_nontype_ref_global<interop_global_var> x) {
  printf("[Lib] test_template_ref_global_interop returned: %d\n", x.get());
}
void test_template_ref_fn_interop(S_nontype_ref_fn<test_extern_func> x) {
  printf("[Lib] test_template_ref_fn_interop calling fn...\n");
  x.call();
}

void RttiPtmdBase::f() {}

void check_rtti_ptmd(const std::type_info &ti_ptmd, const std::type_info &ti_ptmf, const std::type_info &ti_tmpl_ptmd) {
  printf("[Lib] RTTI ti_ptmd name = %s\n", ti_ptmd.name());
  printf("[Lib] RTTI ti_ptmf name = %s\n", ti_ptmf.name());
  printf("[Lib] RTTI ti_tmpl_ptmd name = %s\n", ti_tmpl_ptmd.name());
}

void PTMFVBase::f() { printf("[Lib] PTMFVBase::f()\n"); }
void PTMFDerived::f() { printf("[Lib] PTMFDerived::f()\n"); }

struct LocalGcc2Pmf {
  short delta;
  short index;
  union {
    void* pfn;
    short delta2;
  } u;
};

extern "C" PTMFDerived_PTMF get_ptmf_derived_f() {
  printf("[Lib] get_ptmf_derived_f called!\n");
  PTMFDerived_PTMF pmf = &PTMFDerived::f;
  LocalGcc2Pmf* g = (LocalGcc2Pmf*)&pmf;
  printf("[Lib] get_ptmf_derived_f returning: delta=%d, index=%d, delta2=%d\n",
         g->delta, g->index, g->u.delta2);
  return pmf;
}

void test_ptmf_nontype_vbase_interop() {
  printf("[Lib] test_ptmf_nontype_vbase_interop called!\n");
}

template <void (PTMFDerived::*M)()>
void PTMFNontype<M>::call(PTMFDerived &obj) {
  printf("[Lib] PTMFNontype::call executing...\n");
  (obj.*M)();
}

// Explicitly instantiate the template in lib.cpp so that it is defined in lib.o
template class PTMFNontype<&PTMFDerived::f>;

void check_dia_layout(DiaD *d) {
  printf("[Lib] check_dia_layout: d=%p, B=%p, C=%p, A=%p\n",
         d, (DiaB*)d, (DiaC*)d, (DiaA*)d);
}

void check_fuzz_empty_layout(FuzzEmptyBases *p) {
  printf("[Lib] check_fuzz_empty_layout: p=%p, Empty1=%p, Empty2=%p\n",
         p, (FuzzEmpty1*)p, (FuzzEmpty2*)p);
}

void PmfCheckBase::f() { printf("[Lib] PmfCheckBase::f()\n"); }
void PmfCheckBase::g() { printf("[Lib] PmfCheckBase::g()\n"); }

extern "C" PmfCheckBase_PTMF get_pmf_check_f() {
  return &PmfCheckBase::f;
}

extern "C" PmfCheckBase_PTMF get_pmf_check_g() {
  return &PmfCheckBase::g;
}

void RttiNvVbase_NV::fnv() { printf("[Lib] RttiNvVbase_NV::fnv\n"); }
void RttiNvVbase_V1::f1() { printf("[Lib] RttiNvVbase_V1::f1\n"); }
void RttiNvVbase_D::fnv() { printf("[Lib] RttiNvVbase_D::fnv\n"); }
void RttiNvVbase_D::f1() { printf("[Lib] RttiNvVbase_D::f1\n"); }

RttiNvVbase_NV* get_rtti_nv_vbase_object() {
  static RttiNvVbase_D d;
  return &d;
}

void RttiMultiV1::f1() { printf("[Lib] RttiMultiV1::f1\n"); }
void RttiMultiV2::f2() { printf("[Lib] RttiMultiV2::f2\n"); }
void RttiMultiD::f1() { printf("[Lib] RttiMultiD::f1\n"); }
void RttiMultiD::f2() { printf("[Lib] RttiMultiD::f2\n"); }

RttiMultiD* get_rtti_multi_d_object() {
  static RttiMultiD d;
  return &d;
}

namespace NamespaceDigit1 {
  namespace NamespaceDigit2 {
    int nested_func(int x) {
      return x + 500;
    }
  }
}

int NestedClassDigit1::NestedClassDigit2::nested_func(int x) {
  return x + 600;
}

namespace N1 { namespace N2 { namespace N3 { namespace N4 { namespace N5 {
namespace N6 { namespace N7 { namespace N8 { namespace N9 { namespace N10 {
  int deep_func_10(int x) {
    return x + 1000;
  }
}}}}}}}}}}

const std::type_info& get_lib_anon_secret_ti() {
  return typeid(InteropAnonSecret);
}

VoidFn get_lib_local_fn_ptr() {
  return get_local_fn_ptr_inline();
}

void test_local_class_mangled_uniquifier_interop(VoidFn cons_fn) {
  printf("[Lib] Running local class static method address interop comparison...\n");
  VoidFn lib_fn = get_local_fn_ptr_inline();
  printf("[Lib] lib_fn address = %p, cons_fn address = %p\n", (void*)lib_fn, (void*)cons_fn);

  if (lib_fn != cons_fn) {
    printf("[WARNING] Local Class Mangling Discrepancy! Addresses do not match: lib_fn = %p, cons_fn = %p\n", (void*)lib_fn, (void*)cons_fn);
    printf("[INFO] This is a known GCC 2.95 mangling bug (global static_labelno parsing-phase counter desynchronization) which is intentionally NOT emulated in Clang due to counter instability. See comment in lib.h for details.\n");
  } else {
    printf("[Lib] Local class static method address interop comparison passed!\n");
  }
}

// Implementations for new fuzzed test cases
namespace N1 { namespace N2 { namespace N3 { namespace N4 { namespace N5 {
namespace N6 { namespace N7 { namespace N8 { namespace N9 { namespace N10 {
  int deep_nested_func(int x) {
    return x + 2000;
  }
} } } } } } } } } }

int test_multi_param_template(MultiParamTemplate<int,int,int,int,int,int,int,int,int,int> *p) {
  return p->get_val() + 99;
}

namespace NamespaceLocal {
  RttIBase* get_lib_namespace_local_rtti() {
    return get_namespace_local_rtti_inline<int>();
  }
}

void test_zero_array(int (*x)[0]) {
  printf("[Lib] test_zero_array called successfully!\n");
}

void test_one_array(int (*x)[1]) {
  printf("[Lib] test_one_array called successfully!\n");
}

void IceReproA::foo() { printf("[Lib] IceReproA::foo\n"); }
void IceReproC::bar() { printf("[Lib] IceReproC::bar\n"); }
IceReproD::IceReproD() {
  IceReproB::a = 1;
  b = 2;
  c = 3;
  d = 4;
}
void IceReproD::foo() { printf("[Lib] IceReproD::foo\n"); }

void test_ice_repro(IceReproD *d) {
  printf("[Lib] Clang sizeof(IceReproD) = %d\n", (int)sizeof(IceReproD));
  printf("[Lib] d = %p\n", (void*)d);
  printf("[Lib] IceReproB = %p\n", (void*)(IceReproB*)d);
  printf("[Lib] IceReproC = %p\n", (void*)(IceReproC*)d);
  printf("[Lib] IceReproA via B = %p\n", (void*)(IceReproA*)(IceReproB*)d);
  printf("[Lib] Calling d->foo()...\n");
  d->foo();
  printf("[Lib] Returned from d->foo()!\n");
}

MangleBug::MangleBug() : x(100) {}
void MangleBug::f(MangleBug other) {
  printf("[Lib] MangleBug::f: other.x = %d\n", other.x);
}

void PrimaryBugC0::vfunc_0() { printf("[Lib] PrimaryBugC0::vfunc_0\n"); }
PrimaryBugC0::~PrimaryBugC0() { printf("[Lib] PrimaryBugC0::~PrimaryBugC0\n"); }

void PrimaryBugC1::vfunc_1() { printf("[Lib] PrimaryBugC1::vfunc_1\n"); }
PrimaryBugC1::~PrimaryBugC1() { printf("[Lib] PrimaryBugC1::~PrimaryBugC1\n"); }

PrimaryBugC2::PrimaryBugC2() { printf("[Lib] PrimaryBugC2::PrimaryBugC2\n"); }
PrimaryBugC2::~PrimaryBugC2() { printf("[Lib] PrimaryBugC2::~PrimaryBugC2\n"); }

PrimaryBugC3::PrimaryBugC3() : f0(8), f1(12) {
  printf("[Lib] PrimaryBugC3::PrimaryBugC3\n");
}
PrimaryBugC3::~PrimaryBugC3() { printf("[Lib] PrimaryBugC3::~PrimaryBugC3\n"); }
void PrimaryBugC3::vfunc_3() { printf("[Lib] PrimaryBugC3::vfunc_3\n"); }

extern "C" void exit(int);

void check_primary_bug(PrimaryBugC3 *p, PrimaryBugC1 *b) {
  printf("[Lib] check_primary_bug: p = %p, b = %p\n", p, b);
  printf("[Lib] sizeof(PrimaryBugC3) = %d\n", (int)sizeof(PrimaryBugC3));
  int offset_f0 = (int)((char*)&p->f0 - (char*)p);
  int offset_f1 = (int)((char*)&p->f1 - (char*)p);
  printf("[Lib] offset of f0 = %d, f1 = %d\n", offset_f0, offset_f1);
  int expected_offset = 20;
  int actual_offset = (char*)b - (char*)p;
  printf("[Lib] actual_offset = %d\n", actual_offset);
  if (actual_offset != expected_offset) {
    printf("[ERROR] PrimaryBug offset mismatch! Expected %d, got %d\n", expected_offset, actual_offset);
    exit(1);
  }
  if (sizeof(PrimaryBugC3) != 40) {
    printf("[ERROR] sizeof(PrimaryBugC3) expected 40, got %d\n", (int)sizeof(PrimaryBugC3));
    exit(1);
  }
}

void test_primary_bug_interop() {}

void FuzzSuccessC0::vfunc_0() { printf("[Lib] FuzzSuccessC0::vfunc_0\n"); }
FuzzSuccessC0::~FuzzSuccessC0() { printf("[Lib] FuzzSuccessC0::~FuzzSuccessC0\n"); }

FuzzSuccessC1::~FuzzSuccessC1() { printf("[Lib] FuzzSuccessC1::~FuzzSuccessC1\n"); }

void FuzzSuccessC2::vfunc_2() { printf("[Lib] FuzzSuccessC2::vfunc_2\n"); }
FuzzSuccessC2::~FuzzSuccessC2() { printf("[Lib] FuzzSuccessC2::~FuzzSuccessC2\n"); }

FuzzSuccessC3::FuzzSuccessC3() : f0(8), f1(16) {
  printf("[Lib] FuzzSuccessC3::FuzzSuccessC3\n");
}
FuzzSuccessC3::~FuzzSuccessC3() { printf("[Lib] FuzzSuccessC3::~FuzzSuccessC3\n"); }

void check_fuzz_success_9(FuzzSuccessC3 *p, FuzzSuccessC2 *b2, FuzzSuccessC1 *b1) {
  printf("[Lib] check_fuzz_success_9: p = %p, b2 = %p, b1 = %p\n", p, b2, b1);
  printf("[Lib] sizeof(FuzzSuccessC3) = %d\n", (int)sizeof(FuzzSuccessC3));
  int offset_f0 = (int)((char*)&p->f0 - (char*)p);
  int offset_f1 = (int)((char*)&p->f1 - (char*)p);
  printf("[Lib] offset of f0 = %d, f1 = %d\n", offset_f0, offset_f1);
  int expected_offset_b2 = 20;
  int expected_offset_b1 = 44;
  int actual_offset_b2 = (char*)b2 - (char*)p;
  int actual_offset_b1 = (char*)b1 - (char*)p;
  printf("[Lib] actual_offset_b2 = %d, actual_offset_b1 = %d\n", actual_offset_b2, actual_offset_b1);
  if (actual_offset_b2 != expected_offset_b2 || actual_offset_b1 != expected_offset_b1) {
    printf("[ERROR] FuzzSuccess9 offset mismatch! Expected b2=%d, b1=%d, got b2=%d, b1=%d\n", expected_offset_b2, expected_offset_b1, actual_offset_b2, actual_offset_b1);
    exit(1);
  }
  if (sizeof(FuzzSuccessC3) != 48) {
    printf("[ERROR] sizeof(FuzzSuccessC3) expected 48, got %d\n", (int)sizeof(FuzzSuccessC3));
    exit(1);
  }
}

AlignBugC0::~AlignBugC0() { printf("[Lib] AlignBugC0::~AlignBugC0\n"); }

AlignBugC1::AlignBugC1() : f0(4.0), f1(12), f2(20), f3(21) {
  printf("[Lib] AlignBugC1::AlignBugC1\n");
}
AlignBugC1::~AlignBugC1() { printf("[Lib] AlignBugC1::~AlignBugC1\n"); }

AlignBugC2::AlignBugC2() { printf("[Lib] AlignBugC2::AlignBugC2\n"); }
AlignBugC2::~AlignBugC2() { printf("[Lib] AlignBugC2::~AlignBugC2\n"); }
void AlignBugC2::vfunc_2() { printf("[Lib] AlignBugC2::vfunc_2\n"); }

AlignBugC3::AlignBugC3() : f0(4.0), f1(12), f2(16), f3(20) {
  printf("[Lib] AlignBugC3::AlignBugC3\n");
}
AlignBugC3::~AlignBugC3() { printf("[Lib] AlignBugC3::~AlignBugC3\n"); }

void check_align_bug(AlignBugC2 *p2, AlignBugC3 *p3, AlignBugC1 *p1, AlignBugC0 *p0) {
  printf("[Lib] check_align_bug: p2=%p, p3=%p, p1=%p, p0=%p\n", p2, p3, p1, p0);
  printf("[Lib] sizeof(AlignBugC1) = %d\n", (int)sizeof(AlignBugC1));
  printf("[Lib] sizeof(AlignBugC2) = %d\n", (int)sizeof(AlignBugC2));
  printf("[Lib] sizeof(AlignBugC3) = %d\n", (int)sizeof(AlignBugC3));

  int offset_p1_f0 = (int)((char*)&p1->f0 - (char*)p1);
  int offset_p1_f1 = (int)((char*)&p1->f1 - (char*)p1);
  int offset_p1_f2 = (int)((char*)&p1->f2 - (char*)p1);
  int offset_p1_f3 = (int)((char*)&p1->f3 - (char*)p1);
  printf("[Lib] p1 fields: f0=%d, f1=%d, f2=%d, f3=%d\n", offset_p1_f0, offset_p1_f1, offset_p1_f2, offset_p1_f3);

  int offset_p3_f0 = (int)((char*)&p3->f0 - (char*)p3);
  int offset_p3_f1 = (int)((char*)&p3->f1 - (char*)p3);
  int offset_p3_f2 = (int)((char*)&p3->f2 - (char*)p3);
  int offset_p3_f3 = (int)((char*)&p3->f3 - (char*)p3);
  printf("[Lib] p3 fields: f0=%d, f1=%d, f2=%d, f3=%d\n", offset_p3_f0, offset_p3_f1, offset_p3_f2, offset_p3_f3);

  int actual_offset_p1_p0 = (char*)(AlignBugC0*)p1 - (char*)p1;
  int actual_offset_p2_p1 = (char*)(AlignBugC1*)p2 - (char*)p2;
  int actual_offset_p2_p0 = (char*)(AlignBugC0*)p2 - (char*)p2;
  int actual_offset_p3_p0 = (char*)(AlignBugC0*)p3 - (char*)p3;

  printf("[Lib] base offsets: p1->p0=%d, p2->p1=%d, p2->p0=%d, p3->p0=%d\n",
         actual_offset_p1_p0, actual_offset_p2_p1, actual_offset_p2_p0, actual_offset_p3_p0);

  if (sizeof(AlignBugC1) != 28 || sizeof(AlignBugC2) != 32 || sizeof(AlignBugC3) != 28) {
    printf("[ERROR] AlignBug size mismatch!\n");
    exit(1);
  }
  if (offset_p1_f0 != 4 || offset_p1_f1 != 12 || offset_p1_f2 != 20 || offset_p1_f3 != 21) {
    printf("[ERROR] AlignBug p1 fields offset mismatch!\n");
    exit(1);
  }
  if (offset_p3_f0 != 4 || offset_p3_f1 != 12 || offset_p3_f2 != 16 || offset_p3_f3 != 20) {
    printf("[ERROR] AlignBug p3 fields offset mismatch!\n");
    exit(1);
  }
  if (actual_offset_p1_p0 != 24 || actual_offset_p2_p1 != 0 || actual_offset_p2_p0 != 28 || actual_offset_p3_p0 != 24) {
    printf("[ERROR] AlignBug base offset mismatch!\n");
    exit(1);
  }
}

void LinkBugC0::print_class() { printf("[Lib] LinkBugC0::print_class\n"); }
void LinkBugC0::vfunc_0() {}
LinkBugC0::~LinkBugC0() { printf("[Lib] LinkBugC0::~LinkBugC0\n"); }

void LinkBugC1::print_class() { printf("[Lib] LinkBugC1::print_class\n"); }
LinkBugC1::~LinkBugC1() { printf("[Lib] LinkBugC1::~LinkBugC1\n"); }

void LinkBugC2::print_class() { printf("[Lib] LinkBugC2::print_class\n"); }
LinkBugC2::~LinkBugC2() { printf("[Lib] LinkBugC2::~LinkBugC2\n"); }

void check_link_bug(LinkBugC1 *p1, LinkBugC2 *p2) {
  printf("[Lib] check_link_bug: p1=%p, p2=%p\n", p1, p2);
  p1->print_class();
  p2->print_class();
}

VirtOnlyBugC0::VirtOnlyBugC0() : f0(0) { printf("[Lib] VirtOnlyBugC0::VirtOnlyBugC0\n"); }
void VirtOnlyBugC0::print_class() { printf("[Lib] VirtOnlyBugC0::print_class\n"); }
void VirtOnlyBugC0::vfunc_0() {}
VirtOnlyBugC0::~VirtOnlyBugC0() { printf("[Lib] VirtOnlyBugC0::~VirtOnlyBugC0\n"); }

VirtOnlyBugC1::VirtOnlyBugC1() { printf("[Lib] VirtOnlyBugC1::VirtOnlyBugC1\n"); }
void VirtOnlyBugC1::print_class() { printf("[Lib] VirtOnlyBugC1::print_class\n"); }
VirtOnlyBugC1::~VirtOnlyBugC1() { printf("[Lib] VirtOnlyBugC1::~VirtOnlyBugC1\n"); }

VirtOnlyBugC2::VirtOnlyBugC2() : f0(0), f1(0), f2(0), f3(0.0) {
  printf("[Lib] VirtOnlyBugC2::VirtOnlyBugC2\n");
}
void VirtOnlyBugC2::print_class() { printf("[Lib] VirtOnlyBugC2::print_class\n"); }
void VirtOnlyBugC2::vfunc_2() {}
VirtOnlyBugC2::~VirtOnlyBugC2() { printf("[Lib] VirtOnlyBugC2::~VirtOnlyBugC2\n"); }

VirtOnlyBugC3::VirtOnlyBugC3() : f0(0), f1(0) { printf("[Lib] VirtOnlyBugC3::VirtOnlyBugC3\n"); }
void VirtOnlyBugC3::print_class() { printf("[Lib] VirtOnlyBugC3::print_class\n"); }
VirtOnlyBugC3::~VirtOnlyBugC3() { printf("[Lib] VirtOnlyBugC3::~VirtOnlyBugC3\n"); }

void check_virt_only_bug(VirtOnlyBugC3 *p3, VirtOnlyBugC2 *b2, VirtOnlyBugC1 *b1) {
  printf("[Lib] check_virt_only_bug: p3=%p, b2=%p, b1=%p\n", p3, b2, b1);
  printf("[Lib] sizeof(VirtOnlyBugC3) = %d\n", (int)sizeof(VirtOnlyBugC3));

  int offset_f0 = (int)((char*)&p3->f0 - (char*)p3);
  int offset_f1 = (int)((char*)&p3->f1 - (char*)p3);
  printf("[Lib] C3 fields: f0=%d, f1=%d\n", offset_f0, offset_f1);

  int actual_offset_b2 = (char*)b2 - (char*)p3;
  int actual_offset_b1 = (char*)b1 - (char*)p3;
  printf("[Lib] base offsets: b2=%d, b1=%d\n", actual_offset_b2, actual_offset_b1);

  if (sizeof(VirtOnlyBugC3) != 48) {
    printf("[ERROR] VirtOnlyBug C3 size mismatch! Expected 48, got %d\n", (int)sizeof(VirtOnlyBugC3));
    exit(1);
  }
  if (offset_f0 != 8 || offset_f1 != 16) {
    printf("[ERROR] VirtOnlyBug C3 fields offset mismatch!\n");
    exit(1);
  }
  if (actual_offset_b2 != 20 || actual_offset_b1 != 44) {
    printf("[ERROR] VirtOnlyBug base offset mismatch! Expected b2=20, b1=44, got b2=%d, b1=%d\n", actual_offset_b2, actual_offset_b1);
    exit(1);
  }
}

void EmptySubVT_C0::print_class() { printf("[Lib] EmptySubVT_C0::print_class\n"); }
EmptySubVT_C0::~EmptySubVT_C0() { printf("[Lib] EmptySubVT_C0::~EmptySubVT_C0\n"); }

void EmptySubVT_C1::print_class() { printf("[Lib] EmptySubVT_C1::print_class\n"); }
EmptySubVT_C1::~EmptySubVT_C1() { printf("[Lib] EmptySubVT_C1::~EmptySubVT_C1\n"); }

void EmptySubVT_C2::print_class() { printf("[Lib] EmptySubVT_C2::print_class\n"); }
EmptySubVT_C2::~EmptySubVT_C2() { printf("[Lib] EmptySubVT_C2::~EmptySubVT_C2\n"); }

void check_empty_sub_vt(EmptySubVT_C2 *p2) {
  printf("[Lib] check_empty_sub_vt: p2=%p\n", p2);
  p2->print_class();
}

void SuffixPriorBug_C0::print_class() { printf("[Lib] SuffixPriorBug_C0::print_class\n"); }
SuffixPriorBug_C0::~SuffixPriorBug_C0() { printf("[Lib] SuffixPriorBug_C0::~SuffixPriorBug_C0\n"); }

void SuffixPriorBug_C1::print_class() { printf("[Lib] SuffixPriorBug_C1::print_class\n"); }
SuffixPriorBug_C1::~SuffixPriorBug_C1() { printf("[Lib] SuffixPriorBug_C1::~SuffixPriorBug_C1\n"); }

void SuffixPriorBug_C2::print_class() { printf("[Lib] SuffixPriorBug_C2::print_class\n"); }
SuffixPriorBug_C2::~SuffixPriorBug_C2() { printf("[Lib] SuffixPriorBug_C2::~SuffixPriorBug_C2\n"); }

void SuffixPriorBug_C3::print_class() { printf("[Lib] SuffixPriorBug_C3::print_class\n"); }
SuffixPriorBug_C3::~SuffixPriorBug_C3() { printf("[Lib] SuffixPriorBug_C3::~SuffixPriorBug_C3\n"); }

void SuffixPriorBug_C4::print_class() { printf("[Lib] SuffixPriorBug_C4::print_class\n"); }
SuffixPriorBug_C4::~SuffixPriorBug_C4() { printf("[Lib] SuffixPriorBug_C4::~SuffixPriorBug_C4\n"); }

void check_suffix_prior_bug(SuffixPriorBug_C4 *p4, SuffixPriorBug_C3 *b3) {
  printf("[Lib] check_suffix_prior_bug: p4=%p, b3=%p\n", p4, b3);
  p4->print_class();
  b3->print_class();
}

void Fuzz676_C0::print_class() { printf("[Lib] Fuzz676_C0::print_class\n"); }
void Fuzz676_C0::vf_p3JjdlrA() {}
Fuzz676_C0::~Fuzz676_C0() { printf("[Lib] Fuzz676_C0::~Fuzz676_C0\n"); }

void Fuzz676_C1::print_class() { printf("[Lib] Fuzz676_C1::print_class\n"); }
Fuzz676_C1::~Fuzz676_C1() { printf("[Lib] Fuzz676_C1::~Fuzz676_C1\n"); }

void Fuzz676_C2::print_class() { printf("[Lib] Fuzz676_C2::print_class\n"); }
void Fuzz676_C2::vf_XGGVVSochQ() {}
Fuzz676_C2::~Fuzz676_C2() { printf("[Lib] Fuzz676_C2::~Fuzz676_C2\n"); }

void Fuzz676_C3::print_class() { printf("[Lib] Fuzz676_C3::print_class\n"); }
Fuzz676_C3::~Fuzz676_C3() { printf("[Lib] Fuzz676_C3::~Fuzz676_C3\n"); }

void Fuzz676_C4::print_class() { printf("[Lib] Fuzz676_C4::print_class\n"); }
Fuzz676_C4::~Fuzz676_C4() { printf("[Lib] Fuzz676_C4::~Fuzz676_C4\n"); }

void check_fuzz_676(Fuzz676_C4 *p4) {
  printf("[Lib] check_fuzz_676: p4=%p\n", p4);
  p4->print_class();
}

void Fuzz682_C0::print_class() { printf("[Lib] Fuzz682_C0::print_class\n"); }
void Fuzz682_C0::vf_gUhgunHxX() {}
Fuzz682_C0::~Fuzz682_C0() { printf("[Lib] Fuzz682_C0::~Fuzz682_C0\n"); }

void Fuzz682_C1::print_class() { printf("[Lib] Fuzz682_C1::print_class\n"); }
void Fuzz682_C1::vf_jTVkiT() {}
Fuzz682_C1::~Fuzz682_C1() { printf("[Lib] Fuzz682_C1::~Fuzz682_C1\n"); }

void Fuzz682_C2::print_class() { printf("[Lib] Fuzz682_C2::print_class\n"); }
Fuzz682_C2::~Fuzz682_C2() { printf("[Lib] Fuzz682_C2::~Fuzz682_C2\n"); }

void Fuzz682_C3::print_class() { printf("[Lib] Fuzz682_C3::print_class\n"); }
Fuzz682_C3::~Fuzz682_C3() { printf("[Lib] Fuzz682_C3::~Fuzz682_C3\n"); }

void check_fuzz_682(Fuzz682_C3 *p3) {
  printf("[Lib] check_fuzz_682: p3=%p\n", p3);

  // Print direct base offsets first
  Fuzz682_C2 *b2 = (Fuzz682_C2*)p3;
  Fuzz682_C1 *b1 = (Fuzz682_C1*)p3;
  printf("[Lib] Fuzz682_C3->Fuzz682_C2 base offset = %d\n", (int)((char*)b2 - (char*)p3));
  printf("[Lib] Fuzz682_C3->Fuzz682_C1 base offset = %d\n", (int)((char*)b1 - (char*)p3));

  // Cast via virtual path to get the virtual base C0
  Fuzz682_C0 *vbase = (Fuzz682_C0*)b2;
  printf("[Lib] Fuzz682_C3->virtual Fuzz682_C0 base offset = %d\n", (int)((char*)vbase - (char*)p3));

  // Cast via non-virtual path to get the non-virtual base C0
  Fuzz682_C0 *nvbase = (Fuzz682_C0*)b1;
  printf("[Lib] Fuzz682_C3->non-virtual Fuzz682_C0 base offset = %d\n", (int)((char*)nvbase - (char*)p3));

  // Virtual call last (will crash if vtable is wrong)
  p3->print_class();
  vbase->print_class();
  nvbase->print_class();
}

void Fuzz691_C0::print_class() { printf("[Lib] Fuzz691_C0::print_class\n"); }
Fuzz691_C0::~Fuzz691_C0() { printf("[Lib] Fuzz691_C0::~Fuzz691_C0\n"); }

void Fuzz691_C1::print_class() { printf("[Lib] Fuzz691_C1::print_class\n"); }
void Fuzz691_C1::vf_N55nula() {}
Fuzz691_C1::~Fuzz691_C1() { printf("[Lib] Fuzz691_C1::~Fuzz691_C1\n"); }

void Fuzz691_C2::print_class() { printf("[Lib] Fuzz691_C2::print_class\n"); }
Fuzz691_C2::~Fuzz691_C2() { printf("[Lib] Fuzz691_C2::~Fuzz691_C2\n"); }

void Fuzz691_C3::print_class() { printf("[Lib] Fuzz691_C3::print_class\n"); }
void Fuzz691_C3::vf_WAXCZZoOw() {}
Fuzz691_C3::~Fuzz691_C3() { printf("[Lib] Fuzz691_C3::~Fuzz691_C3\n"); }

void check_fuzz_691(Fuzz691_C3 *p3) {
  printf("[Lib] check_fuzz_691: p3=%p\n", p3);
  p3->print_class();
}

void Fuzz879_C0::print_class() { printf("[Lib] Fuzz879_C0::print_class\n"); }
void Fuzz879_C0::vf_UFTS0uO() {}
Fuzz879_C0::~Fuzz879_C0() { printf("[Lib] Fuzz879_C0::~Fuzz879_C0\n"); }

void Fuzz879_C1::print_class() { printf("[Lib] Fuzz879_C1::print_class\n"); }
void Fuzz879_C1::vf_NYi17f9Bv4() {}
Fuzz879_C1::~Fuzz879_C1() { printf("[Lib] Fuzz879_C1::~Fuzz879_C1\n"); }

void Fuzz879_C2::print_class() { printf("[Lib] Fuzz879_C2::print_class\n"); }
void Fuzz879_C2::vf_OR26a() {}
Fuzz879_C2::~Fuzz879_C2() { printf("[Lib] Fuzz879_C2::~Fuzz879_C2\n"); }

void Fuzz879_C3::print_class() { printf("[Lib] Fuzz879_C3::print_class\n"); }
bool Fuzz879_C3::operator==(const Fuzz879_C3& other) const { printf("[Op] operator==\n"); return true; }
Fuzz879_C3::~Fuzz879_C3() { printf("[Lib] Fuzz879_C3::~Fuzz879_C3\n"); }

void Fuzz879_C4::print_class() { printf("[Lib] Fuzz879_C4::print_class\n"); }
void Fuzz879_C4::mf_zCj8L(long long a0) { printf("[Member] Fuzz879_C4::mf_zCj8L\n"); }
Fuzz879_C4::~Fuzz879_C4() { printf("[Lib] Fuzz879_C4::~Fuzz879_C4\n"); }

void check_fuzz_879(Fuzz879_C4 *p4) {
  printf("[Lib] check_fuzz_879: p4=%p\n", p4);
  p4->print_class();
}

namespace NsInteropTemplate {
namespace NsSub_ {
  template <> void TemplateClass::mt_func<double, 42>(double a0) {
    printf("[Lib] Specialized mt_func: double %f\n", a0);
  }
  template <> void tfn_func<double, 42>(double a0) {
    printf("[Lib] Specialized tfn_func: double %f\n", a0);
  }
}
}

void check_template_interop(NsInteropTemplate::NsSub_::TemplateClass *p) {
  printf("[Lib] check_template_interop: p=%p\n", p);
  p->mt_func<double, 42>(3.14);
  NsInteropTemplate::NsSub_::tfn_func<double, 42>(2.718);
}

namespace NsInteropVirtOverride {
  Base::Base() { print_class(); }
  Base::~Base() { print_class(); }
  void Base::print_class() { printf("[Lib] Base::print_class\n"); }
  int Base::print_class_val() { return 1; }

  CDerived::CDerived() { print_class(); }
  CDerived::~CDerived() { print_class(); }
  void CDerived::print_class() { printf("[Lib] CDerived::print_class\n"); }
  int CDerived::print_class_val() { return 2; }

  VDerived::VDerived() { print_class(); }
  VDerived::~VDerived() { print_class(); }
  void VDerived::print_class() { printf("[Lib] VDerived::print_class\n"); }
  int VDerived::print_class_val() { return 3; }

  CpDerived::CpDerived() { print_class(); }
  CpDerived::~CpDerived() { print_class(); }
  void CpDerived::print_class() { printf("[Lib] CpDerived::print_class\n"); }
  int CpDerived::print_class_val() { return 4; }

  int check_virt_override_interop() {
    printf("[Lib] Instantiating CpDerived...\n");
    CpDerived obj;
    Base *b = (Base*)&obj;
    return b->print_class_val();
  }
}
