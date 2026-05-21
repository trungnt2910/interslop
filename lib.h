#ifndef LIB_H
#define LIB_H

extern "C" int printf(const char*, ...);

class Base1 {
public:
  Base1();
  virtual ~Base1();
  virtual int f1();
};

class Base2 {
public:
  Base2();
  virtual ~Base2();
  virtual int f2();
};

class Derived : public Base1, public Base2 {
public:
  static int instance_count;
  int z;

  Derived(int z);
  Derived(const Derived &other);
  virtual ~Derived();
  virtual int f1();
  virtual int f2();
  virtual int f3() const;
  virtual int f4();
  void operator+=(int val);
  void operator-=(int val);
  void operator*=(int val);
  void operator/=(int val);
  void operator%=(int val);
  void operator&=(int val);
  void operator|=(int val);
  void operator^=(int val);
  void operator<<=(int val);
  void operator>>=(int val);
  operator int();
};

int normal_func(int x, double y);
void trigger_eh_from_lib();
void test_consumer_eh();
void trigger_eh_base();
void test_consumer_eh_base();
void trigger_eh_vbase();
void test_consumer_eh_vbase();
void trigger_eh_int_ptr();
void trigger_eh_fn_ptr();
void test_consumer_eh_fn_ptr();
void test_const_ptr(int * const * p);

typedef int (Derived::*DerivedPTMF)();
typedef int Derived::*DerivedPTMD;

void test_const_ptmd(DerivedPTMD const * p);
void test_const_ptmf(DerivedPTMF const * p);
void test_const_ptr_ref(int * const & p);
void test_const_ptmd_ref(DerivedPTMD const & p);
void test_const_ptmf_ref(DerivedPTMF const & p);
void test_cv_ptr(int * const volatile * p);
void test_complex(__complex__ double c);
void test_array_ref(int (&arr)[10]);
void test_fn_ptr(int (*fn)(double));
void test_vu_ptr(volatile unsigned int * p);
void test_restrict_ptr(int * __restrict * p);

DerivedPTMF get_lib_ptmf(int which);
int call_lib_ptmf(Derived *d, DerivedPTMF ptmf);

typedef int (Derived::*DerivedConstPTMF)() const;
DerivedConstPTMF get_lib_const_ptmf();
int call_lib_const_ptmf(const Derived *d, DerivedConstPTMF ptmf);

DerivedPTMD get_lib_ptmd();
int get_lib_ptmd_val(Derived *d, DerivedPTMD ptmd);

class CookieTester {
public:
  static int dtor_count;
  int value;
  CookieTester();
  ~CookieTester();
};

CookieTester *alloc_cookie_tester(int n);
void delete_cookie_tester(CookieTester *p);

class VBaseDtorTester {
public:
  static int dtor_count;
  int x;
  VBaseDtorTester();
  virtual ~VBaseDtorTester();
};

class VSubDtorTester : virtual public VBaseDtorTester {
public:
  int y;
  VSubDtorTester();
  virtual ~VSubDtorTester();
};

VSubDtorTester *alloc_vsub();
void delete_vsub(VSubDtorTester *p);

inline __attribute__((always_inline)) int test_inline_static(int v) {
  static int s_var = v;
  return s_var;
}

int call_inline_static_from_lib(int v);

struct ByValStruct {
  int val;
};
int test_pass_by_val(ByValStruct s);

int test_repeat_ptrs(int *a, int *b, int *c);
int test_repeat_bools(bool a, bool b, bool c);
int test_mixed_repeats(int a, int *b, int *c);

template <typename T>
class TemplateClass {
public:
  T value;
  TemplateClass(T val) : value(val) {}
  virtual ~TemplateClass() {}
  virtual T get_value() { return value; }
};

int test_template_mangling(TemplateClass<int> *p);

template <typename T> struct InteropInner {};
template <template <typename> class TT> struct InteropOuter {};
int test_template_template_mangling(InteropOuter<InteropInner> *p);


class PolyBase {
public:
  int p;
  PolyBase();
  virtual ~PolyBase();
  virtual int poly_func();
};

class IntermediateVBase : virtual public PolyBase {
public:
  int i;
  IntermediateVBase();
  virtual ~IntermediateVBase();
  virtual int inter_func();
};

class DeepVSub : virtual public IntermediateVBase {
public:
  int d;
  DeepVSub();
  virtual ~DeepVSub();
};

DeepVSub *alloc_deep_vsub();
void delete_deep_vsub(DeepVSub *p);
void test_deep_vsub_interop();

struct EboBase1 {
  int x;
};
struct EboEmpty {
};
struct EboDerived : EboBase1, EboEmpty {
  int y;
};

void check_ebo_derived(EboDerived *d);

class GuardTester {
public:
  int val;
  GuardTester(int v);
  ~GuardTester();
};

inline __attribute__((always_inline)) int test_inline_guard(int v) {
  static GuardTester s_guard(v);
  return s_guard.val;
}

int call_inline_guard_from_lib(int v);
void test_inline_guard_interop();

struct CovariantBase1 {
  virtual void dummy();
};
struct CovariantBase2 {
  virtual CovariantBase2* clone();
};
struct CovariantDerived : CovariantBase1, CovariantBase2 {
  virtual CovariantDerived* clone();
};

CovariantBase2* get_covariant_derived();

extern int nontype_global_var;
void nontype_global_func();

template <int I> struct S_nontype1 {};
template <int *P> struct S_nontype2 {};
template <void (*F)()> struct S_nontype3 {};
template <int Derived::*M> struct S_nontype4 {};
template <int (Derived::*M)()> struct S_nontype5 {};

void test_nontype_1(S_nontype1<42> x);
void test_nontype_2(S_nontype2<&nontype_global_var> x);
void test_nontype_3(S_nontype3<&nontype_global_func> x);
void test_nontype_4(S_nontype4<&Derived::z> x);
void test_nontype_5(S_nontype5<&Derived::f1> x);
void test_nontype_7(S_nontype5<&Derived::f2> x);

class PureVirtualBase {
public:
  virtual void pure_func() = 0;
  virtual void key_func();
};

struct LayoutBase1 {
  int x;
};
struct __attribute__((aligned(8))) LayoutBase2 {
  double y;
};
struct LayoutDerived1 : LayoutBase1, LayoutBase2 {
  char c;
};
struct LayoutDerived2 : LayoutBase2, LayoutBase1 {
  char c;
};

void check_layout_derived(LayoutDerived1 *d1, LayoutDerived2 *d2);

struct EmptyVBase1 {};
struct EmptyVBase2 {};
struct EmptyVDerived1 : virtual EmptyVBase1 {};
struct EmptyVDerived2 : virtual EmptyVBase1, virtual EmptyVBase2 {};

void check_empty_vderived(EmptyVDerived1 *d1, EmptyVDerived2 *d2);
void check_empty_vderived_array(EmptyVDerived2 *arr, int n);

class VptrNotZero {
public:
  int x;
  VptrNotZero();
  virtual void foo();
};

typedef void (VptrNotZero::*VptrNotZeroPTMF)();
VptrNotZeroPTMF get_vptr_not_zero_ptmf();
class DynBase {
public:
  virtual ~DynBase() {}
};
class DynDerived : public DynBase {
public:
  int val;
  DynDerived(int v);
  virtual ~DynDerived() {}
};
DynBase* get_dyn_derived(int v);

template <typename T>
T interop_fn_tmpl(T val);

struct NonDynamicClass {
  int a;
};

struct ReproClass : NonDynamicClass, virtual EmptyVBase1 {
  int x;
};

void check_repro_class(ReproClass *p);

struct MiBase1 {
  int x;
};
struct MiBase2 {
  virtual void foo();
};
struct MiDerived : MiBase1, MiBase2 {
  virtual void bar();
};

template <void (VptrNotZero::*M)()> struct S_nontype_virt {};
template <void (MiDerived::*M)()> struct S_nontype_mi {};

void test_nontype_virt(S_nontype_virt<&VptrNotZero::foo> x);
void test_nontype_mi(S_nontype_mi<&MiDerived::bar> x);

struct InteropVBase {
  int val;
  InteropVBase();
  virtual ~InteropVBase();
  virtual int vfn();
};

struct InteropVDerived : virtual InteropVBase {
  int val2;
  InteropVDerived(int v1, int v2);
  virtual ~InteropVDerived();
  virtual int vfn();
};

typedef int (InteropVDerived::*VDerivedPTMF)();
VDerivedPTMF get_vderived_ptmf();
int call_vderived_ptmf(InteropVDerived *d, VDerivedPTMF ptmf);
template <typename T>
inline int test_local_class_collision(T val) {
  struct Local {
    T v;
    Local(T v) : v(v) {}
    int get_size() {
      return sizeof(T);
    }
  };
  Local l(val);
  return l.get_size();
}

struct DtorOnlyNonTrivial {
  int val;
  ~DtorOnlyNonTrivial();
};
int test_pass_by_val_dtor_only(DtorOnlyNonTrivial s);

struct OverwriteVBase {
  int v_val;
  OverwriteVBase();
};

struct OverwriteNonVirtualBase {
  int nv_val;
  OverwriteNonVirtualBase();
};

struct OverwriteIntermediate : OverwriteNonVirtualBase, virtual OverwriteVBase {
  int x;
  OverwriteIntermediate();
};

struct OverwriteDerived : OverwriteIntermediate {
  int y;
  OverwriteDerived();
};

void check_overwrite_derived(OverwriteDerived *d);

struct EmptyStruct {};
int test_empty_struct_pass(int a, EmptyStruct e, int b);

struct PTMFCompBase1 {
  virtual void foo() {}
};
struct PTMFCompBase2 {
  virtual void foo() {}
};
struct PTMFCompDerived : PTMFCompBase1, PTMFCompBase2 {
};

typedef void (PTMFCompDerived::*CompDerivedPTMF)();
typedef void (PTMFCompBase2::*CompBase2PTMF)();

void test_ptmf_compare_interop();

struct ReproVBaseNonDyn {
  int x;
};
struct ReproVBaseDyn {
  virtual void f();
};
struct ReproVDerived : virtual ReproVBaseNonDyn, virtual ReproVBaseDyn {
  int y;
};
void test_vptr_retreival_vbase(ReproVDerived *d);

struct ReproNVBase {
  int x;
  virtual void f();
};
struct ReproNVDerived : ReproNVBase {
  int y;
};
void test_vptr_retreival_nvbase(ReproNVDerived *d);

struct BugAlign4 { int x; };
struct BugEmpty1 {};
struct BugEmpty2 {};
struct BugVBase : BugAlign4, virtual BugEmpty1, virtual BugEmpty2 {};
void check_vbase_alignment_bug(BugVBase *p, int expected_size);

struct RTTIBase {
  virtual ~RTTIBase() {}
};

template <typename T>
inline RTTIBase* get_local_rtti_obj() {
  struct Local : RTTIBase {
    virtual ~Local() {}
  };
  return new Local();
}

RTTIBase* get_lib_local_rtti();

template <typename T>
inline int test_spec_local_class(T val) {
  return 0;
}

template <>
inline __attribute__((noinline)) int test_spec_local_class<int>(int val) {
  struct Local {
    int v;
    Local(int v) : v(v) {}
    __attribute__((noinline)) int get_val() { return v; }
  };
  Local l(val);
  return l.get_val();
}


#include <typeinfo>
inline const std::type_info& get_regular_local_rtti2() {
  struct Local1 { virtual ~Local1() {} };
  struct Local2 { virtual ~Local2() {} };
  return typeid(Local2);
}
const std::type_info& get_lib_regular_local_rtti2();

struct PmfVBase {
  virtual void vbase_virt();
  int pad;
};

struct PmfDerived : virtual PmfVBase {
  virtual void derived_virt();
  virtual void vbase_virt();
};


typedef void (PmfDerived::*PmfDerivedPTMF)();

extern "C" PmfDerivedPTMF get_pmf_derived_vbase_virt();
extern "C" PmfDerivedPTMF get_pmf_derived_derived_virt();
extern "C" void call_pmf_derived(PmfDerived *d, PmfDerivedPTMF ptmf);

extern int pmf_called_vbase;
extern int pmf_called_derived;

class EhVBaseCleanupTester : virtual public PolyBase {
public:
  static int dtor_count;
  int val;
  EhVBaseCleanupTester(int v);
  virtual ~EhVBaseCleanupTester();
};

void trigger_eh_cleanup_from_lib();

#endif







