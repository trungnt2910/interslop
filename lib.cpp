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
}

void ReproNVBase::f() {
  printf("[Lib] ReproNVBase::f() called!\n");
}
void test_vptr_retreival_nvbase(ReproNVDerived *d) {
  printf("[Lib] sizeof(ReproNVDerived) = %d\n", (int)sizeof(ReproNVDerived));
  printf("[Lib] offset of ReproNVBase = %d\n", (int)((char*)(ReproNVBase*)d - (char*)d));
  printf("[Lib] ReproNVDerived typeid name = %s\n", typeid(*d).name());
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



