#ifndef LIB_H
#define LIB_H
#include <stddef.h>

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
void delete_vsub_base(VBaseDtorTester *p);

class InlineVBaseDtorTester {
public:
  static int dtor_count;
  InlineVBaseDtorTester();
  virtual ~InlineVBaseDtorTester();
};

class InlineVSubDtorTester : virtual public InlineVBaseDtorTester {
public:
  InlineVSubDtorTester();
  virtual ~InlineVSubDtorTester() {} // INLINE virtual destructor!
};

InlineVSubDtorTester *alloc_inline_vsub();
void delete_inline_vsub_base(InlineVBaseDtorTester *p);

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

class IndirectVBaseA {
public:
  int a;
  IndirectVBaseA();
  virtual ~IndirectVBaseA();
  virtual void foo();
};

class IndirectVBaseC : virtual public IndirectVBaseA {
public:
  int c;
  IndirectVBaseC();
  virtual ~IndirectVBaseC();
};

class IndirectVBaseD : public IndirectVBaseC {
public:
  int d;
  IndirectVBaseD();
  virtual ~IndirectVBaseD();
};

class IndirectVBaseE : virtual public IndirectVBaseD {
public:
  int e;
  IndirectVBaseE();
  virtual ~IndirectVBaseE();

  void* operator new(size_t size) {
    return ::operator new(size + 8);
  }
  void operator delete(void* ptr) {
    ::operator delete(ptr);
  }
};

void test_indirect_vbase_interop();

namespace InteropNS {
  int namespace_func(int x);
  extern int namespace_var;


  template <typename T>
  class TmplClass {
  public:
    T val;
    TmplClass(T v) : val(v) {}
    T get_val() { return val; }
  };

  int test_ns_tmpl_class(TmplClass<int> *p);
}

namespace A {
  namespace B {
    namespace C {
      namespace D {
        int deep_func(int x);
      }
    }
  }
  namespace std {
    int nested_std_func(int x);
  }
}

namespace std {
  extern int std_var;
  int top_level_std_func(int x);

  namespace B {
    int std_nested_func(int x);
  }
  namespace std {
    int nested_std_std_func(int x);
  }
  namespace my_foo {
    int nested_foo_func(int x);
    namespace std {
      int nested_foo_std_func(int x);
    }
  }
}

namespace Foo {
  template <typename T>
  T templ_func(T x) {
    return x + 1;
  }

  int overload(int x);
  double overload(double x);
}

namespace X {
  struct S {
    int val;
  };
}
namespace Y {
  X::S func(int x);
}

namespace M {
  struct S {
    int x;
  };
  typedef int S::*S_PTMD;
  int func(S_PTMD p, S *s);
}

struct BugPMFV {
  virtual void f();
  int v;
  BugPMFV();
};

struct BugPMFA : virtual BugPMFV {
  int a;
  BugPMFA();
};

struct BugPMFB : BugPMFA {
  virtual void f();
  int b;
  BugPMFB();
};

typedef void (BugPMFB::*BugPMFB_PTMF)();

extern "C" BugPMFB_PTMF get_bug_pmf();
void test_indirect_vbase_pmf_interop();

struct CastBugV1 {
  int v1;
  CastBugV1();
  virtual void f1();
  virtual ~CastBugV1();
};

struct CastBugV2 : virtual CastBugV1 {
  int v2;
  CastBugV2();
  virtual void f2();
  virtual ~CastBugV2();
};

struct CastBugD : virtual CastBugV2 {
  int d;
  CastBugD();
  virtual void fd();
  virtual ~CastBugD();
};

struct CastBugOther { int o; };
struct CastBugDD : CastBugOther, virtual CastBugD {
  int dd;
  CastBugDD();
  virtual ~CastBugDD();
};

extern "C" CastBugD* get_cast_bug_d();
extern "C" CastBugV1* get_cast_bug_v1();
void test_vbptr_cast_bug_interop();

struct CastBugPTMD_A {
  int a;
};
struct CastBugPTMD_B {
  int b;
};
struct CastBugPTMD_C : CastBugPTMD_A, CastBugPTMD_B {
  int c;
};

typedef int CastBugPTMD_B::*CastBugPTMD_B_PTMD;
typedef int CastBugPTMD_C::*CastBugPTMD_C_PTMD;

extern "C" CastBugPTMD_C_PTMD get_cast_bug_ptmd_null();
extern "C" CastBugPTMD_C_PTMD get_cast_bug_ptmd_nonnull();
extern "C" int call_cast_bug_ptmd(CastBugPTMD_C *obj, CastBugPTMD_C_PTMD p);
void test_ptmd_cast_null_interop();

extern int interop_global_var;
void test_extern_func();
template <int &R> struct S_nontype_ref_global {
  int get() { return R; }
};
template <void (&F)()> struct S_nontype_ref_fn {
  void call() { F(); }
};
void test_template_ref_global_interop(S_nontype_ref_global<interop_global_var> x);
void test_template_ref_fn_interop(S_nontype_ref_fn<test_extern_func> x);

struct RttiPtmdBase {
  int x;
  void f();
};
template <typename T> struct RttiPtmdTmpl {
  T x;
};
typedef int RttiPtmdBase::*RttiPtmdBase_PTMD;
typedef void (RttiPtmdBase::*RttiPtmdBase_PTMF)();
typedef int RttiPtmdTmpl<int>::*RttiPtmdTmpl_PTMD;

#include <typeinfo>
void check_rtti_ptmd(const std::type_info &ti_ptmd, const std::type_info &ti_ptmf, const std::type_info &ti_tmpl_ptmd);

struct PTMFVBase {
  virtual void f();
};
struct PTMFDerived : virtual PTMFVBase {
  virtual void f();
};
template <void (PTMFDerived::*M)()> struct PTMFNontype {
  void call(PTMFDerived &obj);
};
typedef void (PTMFDerived::*PTMFDerived_PTMF)();
extern "C" PTMFDerived_PTMF get_ptmf_derived_f();
void test_ptmf_nontype_vbase_interop();

struct DiaA { int a; };
struct DiaB : virtual DiaA { int b; };
struct DiaC : virtual DiaA { int c; };
struct DiaD : DiaB, DiaC { int d; };
void check_dia_layout(DiaD *d);

struct FuzzEmpty1 {};
struct FuzzEmpty2 {};
struct FuzzEmptyBases : FuzzEmpty1, FuzzEmpty2 {};
void check_fuzz_empty_layout(FuzzEmptyBases *p);

struct PmfCheckBase {
  virtual void f();
  void g();
};
typedef void (PmfCheckBase::*PmfCheckBase_PTMF)();
extern "C" PmfCheckBase_PTMF get_pmf_check_f();
extern "C" PmfCheckBase_PTMF get_pmf_check_g();
void test_pmf_check_interop();

struct RttiNvVbase_NV {
  virtual void fnv();
};

struct RttiNvVbase_V1 {
  virtual void f1();
};

struct RttiNvVbase_D : RttiNvVbase_NV, virtual RttiNvVbase_V1 {
  void fnv();
  void f1();
};

RttiNvVbase_NV* get_rtti_nv_vbase_object();
void test_rtti_nv_vbase_dynamic_cast_interop();

struct RttiMultiV1 {
  virtual void f1();
};

struct RttiMultiV2 {
  virtual void f2();
};

struct RttiMultiD : virtual RttiMultiV1, virtual RttiMultiV2 {
  int x;
  void f1();
  void f2();
};

RttiMultiD* get_rtti_multi_d_object();
void test_rtti_multiple_vbases_interop();

namespace NamespaceDigit1 {
  namespace NamespaceDigit2 {
    int nested_func(int x);
  }
}

struct NestedClassDigit1 {
  struct NestedClassDigit2 {
    int nested_func(int x);
  };
};

namespace N1 { namespace N2 { namespace N3 { namespace N4 { namespace N5 {
namespace N6 { namespace N7 { namespace N8 { namespace N9 { namespace N10 {
  int deep_func_10(int x);
}}}}}}}}}}

#include <typeinfo>
namespace {
  struct InteropAnonSecret {
    int val;
  };
}
const std::type_info& get_lib_anon_secret_ti();
void test_anon_namespace_rtti_interop();

/*
 * --- GCC 2.95 NAME MANGLING BUG: LOCAL CLASS MEMBER SUFFIX DISCREPANCY ---
 *
 * DESCRIPTION OF THE BUG:
 * Under the legacy GCC 2.95 ABI, any member function (virtual, static, constructor,
 * or destructor) of a local class (a class defined inside a function body) is mangled
 * with an additional numeric suffix like `.` + `static_labelno` at the very end
 * of its symbol name (e.g., `method__Q229get_local_fn_ptr_inline__Fv.0_5Local.100`).
 *
 * HOW IT IS BUGGED IN GCC:
 * 1. GCC 2.95 maintains a global parsing-phase counter `static_labelno` (defined in
 *    `decl.c`) that is reset to 0 inside `start_function()` during code generation.
 * 2. However, when a local class is declared, its member functions are parsed and
 *    mangled during the parsing phase (inside `grokclassfn()`), LONG before code
 *    generation starts for the enclosing function.
 * 3. Consequently, `static_labelno` is never reset during parsing! It acts as a
 *    global, unstable translation-unit-scope counter that grows larger as more
 *    headers and templates are parsed before it.
 *
 * WHY WE CHOOSE NOT TO EMULATE THIS BUG IN CLANG:
 * - The suffix is fundamentally unstable even under GCC 2.95 itself! If two TUs
 *   include the same header but have different system headers included before it,
 *   GCC will mangle the same inline method differently (e.g. `.50` vs `.120`),
 *   violating ODR and failing to merge weak symbols, resulting in runtime address
 *   mismatches or linker errors.
 * - Clang uses a clean, stable, AST-based local class numbering context (`MangleNumberingContext`)
 *   which is 100% stable and ODR-compliant. Attempting to emulate GCC's unstable global
 *   parsing counter in Clang's on-demand AST-based compiler is impossible.
 * - Local classes are strictly scoped, and their member pointers are never naturally
 *   exposed across TUs, so this has zero impact on real-world binary interoperability.
 *
 * VERIFICATION:
 * This test takes the address of the static method of a local class inside an inline
 * function in both GCC and Clang. Because Clang's GCC2 ABI generates suffix-free
 * symbols (stable/standard-compliant) and GCC 2.95 generates unstable suffixes,
 * the linker fails to merge them (ODR violation), resulting in different addresses.
 */
typedef void (*VoidFn)();
static inline VoidFn get_local_fn_ptr_inline() {
  struct Local {
    static void method() {}
  };
  return &Local::method;
}
VoidFn get_lib_local_fn_ptr();
void test_local_class_mangled_uniquifier_interop(VoidFn cons_fn);

// Fuzzed namespace test cases for increased coverage
namespace N1 { namespace N2 { namespace N3 { namespace N4 { namespace N5 {
namespace N6 { namespace N7 { namespace N8 { namespace N9 { namespace N10 {
  int deep_nested_func(int x);
} } } } } } } } } }

template<typename T1, typename T2, typename T3, typename T4, typename T5,
         typename T6, typename T7, typename T8, typename T9, typename T10>
struct MultiParamTemplate {
  T1 val1;
  MultiParamTemplate(T1 v) : val1(v) {}
  T1 get_val() { return val1; }
};
int test_multi_param_template(MultiParamTemplate<int,int,int,int,int,int,int,int,int,int> *p);

namespace NamespaceLocal {
  struct RttIBase {
    virtual ~RttIBase() {}
  };

  template<typename T>
  inline RttIBase* get_namespace_local_rtti_inline() {
    struct Local : RttIBase {
      virtual ~Local() {}
    };
    return new Local();
  }
  RttIBase* get_lib_namespace_local_rtti();
}

void test_zero_array(int (*x)[0]);
void test_one_array(int (*x)[1]);

// Hybrid NV/V inheritance layout ICE repro
struct IceReproA {
  virtual void foo();
  int a;
};

struct IceReproB : IceReproA {
  int b;
};

struct IceReproC : virtual IceReproA {
  int c;
  virtual void bar();
};

struct IceReproD : IceReproB, IceReproC {
  int d;
  IceReproD();
  virtual void foo();
};

void test_ice_repro(IceReproD *d);

struct MangleBug {
  int x;
  MangleBug();
  void f(MangleBug);
};

void test_mangle_bug_interop();

struct PrimaryBugC0 {
  double f0;
  int f1;
  virtual void vfunc_0();
  virtual ~PrimaryBugC0();
};

struct PrimaryBugC1 {
  virtual void vfunc_1();
  virtual ~PrimaryBugC1();
};

struct PrimaryBugC2 : public virtual PrimaryBugC0 {
  PrimaryBugC2();
  virtual ~PrimaryBugC2();
};

struct PrimaryBugC3 : public virtual PrimaryBugC1, public PrimaryBugC2 {
  short f0;
  int f1;
  PrimaryBugC3();
  virtual ~PrimaryBugC3();
  virtual void vfunc_3();
};

void check_primary_bug(PrimaryBugC3 *p, PrimaryBugC1 *b);
void test_primary_bug_interop();

struct FuzzSuccessC0 {
  char f0;
  virtual void vfunc_0();
  virtual ~FuzzSuccessC0();
};
struct FuzzSuccessC1 {
  virtual ~FuzzSuccessC1();
};
struct FuzzSuccessC2 {
  short f0;
  short f1;
  long long f2;
  double f3;
  virtual void vfunc_2();
  virtual ~FuzzSuccessC2();
};
struct FuzzSuccessC3 : public virtual FuzzSuccessC2, public virtual FuzzSuccessC1 {
  long long f0;
  int f1;
  FuzzSuccessC3();
  virtual ~FuzzSuccessC3();
};
void check_fuzz_success_9(FuzzSuccessC3 *p, FuzzSuccessC2 *b2, FuzzSuccessC1 *b1);

struct AlignBugC0 {
  virtual ~AlignBugC0();
};
struct AlignBugC1 : public virtual AlignBugC0 {
  double f0;
  long long f1;
  char f2;
  char f3;
  AlignBugC1();
  virtual ~AlignBugC1();
};
struct AlignBugC2 : public AlignBugC1, public virtual AlignBugC0 {
  AlignBugC2();
  virtual ~AlignBugC2();
  virtual void vfunc_2();
};
struct AlignBugC3 : public virtual AlignBugC0 {
  double f0;
  int f1;
  int f2;
  int f3;
  AlignBugC3();
  virtual ~AlignBugC3();
};
void check_align_bug(AlignBugC2 *p2, AlignBugC3 *p3, AlignBugC1 *p1, AlignBugC0 *p0);

struct LinkBugC0 {
  double f0;
  int f1;
  char f2;
  virtual void print_class();
  virtual void vfunc_0();
  virtual ~LinkBugC0();
};

struct LinkBugC1 : public LinkBugC0 {
  short f0;
  virtual void print_class();
  virtual ~LinkBugC1();
};

struct LinkBugC2 {
  virtual void print_class();
  virtual ~LinkBugC2();
};

void check_link_bug(LinkBugC1 *p1, LinkBugC2 *p2);

struct VirtOnlyBugC0 {
  char f0;
  virtual void print_class();
  virtual void vfunc_0();
  VirtOnlyBugC0();
  virtual ~VirtOnlyBugC0();
};

struct VirtOnlyBugC1 {
  virtual void print_class();
  VirtOnlyBugC1();
  virtual ~VirtOnlyBugC1();
};

struct VirtOnlyBugC2 {
  short f0;
  short f1;
  long long f2;
  double f3;
  virtual void print_class();
  virtual void vfunc_2();
  VirtOnlyBugC2();
  virtual ~VirtOnlyBugC2();
};

struct VirtOnlyBugC3 : public virtual VirtOnlyBugC2, public virtual VirtOnlyBugC1 {
  long long f0;
  int f1;
  virtual void print_class();
  VirtOnlyBugC3();
  virtual ~VirtOnlyBugC3();
};

void check_virt_only_bug(VirtOnlyBugC3 *p3, VirtOnlyBugC2 *b2, VirtOnlyBugC1 *b1);

struct EmptySubVT_C0 {
  virtual void print_class();
  virtual ~EmptySubVT_C0();
};
struct EmptySubVT_C1 : public virtual EmptySubVT_C0 {
  double f0;
  virtual void print_class();
  virtual ~EmptySubVT_C1();
};
struct EmptySubVT_C2 : public EmptySubVT_C1 {
  double f0;
  virtual void print_class();
  virtual ~EmptySubVT_C2();
};
void check_empty_sub_vt(EmptySubVT_C2 *p2);

struct SuffixPriorBug_C0 {
  virtual void print_class();
  virtual ~SuffixPriorBug_C0();
};
struct SuffixPriorBug_C1 {
  virtual void print_class();
  virtual ~SuffixPriorBug_C1();
};
struct SuffixPriorBug_C2 : public SuffixPriorBug_C0, public SuffixPriorBug_C1 {
  virtual void print_class();
  virtual ~SuffixPriorBug_C2();
};
struct SuffixPriorBug_C3 : public SuffixPriorBug_C0 {
  virtual void print_class();
  virtual ~SuffixPriorBug_C3();
};
struct SuffixPriorBug_C4 : public SuffixPriorBug_C2, public SuffixPriorBug_C3 {
  virtual void print_class();
  virtual ~SuffixPriorBug_C4();
};
void check_suffix_prior_bug(SuffixPriorBug_C4 *p4, SuffixPriorBug_C3 *b3);

struct Fuzz676_C0 {
  int f_vaj4T2;
  virtual void print_class();
  virtual void vf_p3JjdlrA();
  virtual ~Fuzz676_C0();
};
struct Fuzz676_C1 : public Fuzz676_C0 {
  int f_JPl17JuE8r;
  short f_nYhAr;
  long long f_I7edvmUB2b;
  virtual void print_class();
  virtual ~Fuzz676_C1();
};
struct Fuzz676_C2 {
  short f_UgxVK71;
  double f_Narzi4IevMp;
  virtual void print_class();
  virtual void vf_XGGVVSochQ();
  virtual ~Fuzz676_C2();
};
struct Fuzz676_C3 : public virtual Fuzz676_C0, public virtual Fuzz676_C2 {
  char f_GtxZpwfH9UC;
  double f_BHv7IsW;
  long long f_DW3RzC;
  virtual void print_class();
  virtual ~Fuzz676_C3();
};
struct Fuzz676_C4 : public Fuzz676_C3, public virtual Fuzz676_C1 {
  short f_lktr;
  int f_QkTo;
  virtual void print_class();
  virtual ~Fuzz676_C4();
};
void check_fuzz_676(Fuzz676_C4 *p4);

struct Fuzz682_C0 {
  char f__cZwEwNX;
  double f_wMz3wgmhl;
  virtual void print_class();
  virtual void vf_gUhgunHxX();
  virtual ~Fuzz682_C0();
};
struct Fuzz682_C1 : public Fuzz682_C0 {
  virtual void print_class();
  virtual void vf_jTVkiT();
  virtual ~Fuzz682_C1();
};
struct Fuzz682_C2 : public virtual Fuzz682_C0 {
  int f_ClXKVPCWe5;
  short f_GbuWxvq6;
  short f_pUVEk5;
  virtual void print_class();
  virtual ~Fuzz682_C2();
};
struct Fuzz682_C3 : public Fuzz682_C2, public Fuzz682_C1 {
  virtual void print_class();
  virtual ~Fuzz682_C3();
};
void check_fuzz_682(Fuzz682_C3 *p3);

// TODO: Fix
struct Fuzz691_C0 {
  int f_xDIddE_O;
  double f_lWBnzAM0;
  double f_p0wx9OPxPbr;
  virtual void print_class();
  virtual ~Fuzz691_C0();
};
struct Fuzz691_C1 {
  short f_nhZWxRA;
  double f_Abhdftzb;
  virtual void print_class();
  virtual void vf_N55nula();
  virtual ~Fuzz691_C1();
};
struct Fuzz691_C2 : public virtual Fuzz691_C0 {
  virtual void print_class();
  virtual ~Fuzz691_C2();
};
struct Fuzz691_C3 : public Fuzz691_C2, public Fuzz691_C1 {
  char f_vMQOK;
  int f_bM4Ta;
  virtual void print_class();
  virtual void vf_WAXCZZoOw();
  virtual ~Fuzz691_C3();
};
void check_fuzz_691(Fuzz691_C3 *p3);

struct Fuzz879_C0 {
  int f_P7C8EN;
  long long f_WHHEaBsXt;
  double f_ekdYZ88;
  virtual void print_class();
  virtual void vf_UFTS0uO();
  virtual ~Fuzz879_C0();
};
struct Fuzz879_C1 {
  virtual void print_class();
  virtual void vf_NYi17f9Bv4();
  virtual ~Fuzz879_C1();
};
struct Fuzz879_C2 : public virtual Fuzz879_C1 {
  virtual void print_class();
  virtual void vf_OR26a();
  virtual ~Fuzz879_C2();
};
struct Fuzz879_C3 : public Fuzz879_C2, public Fuzz879_C0 {
  int f_XY4fOBLucd;
  virtual void print_class();
  bool operator==(const Fuzz879_C3& other) const;
  virtual ~Fuzz879_C3();
};
struct Fuzz879_C4 : public Fuzz879_C3 {
  virtual void print_class();
  void mf_zCj8L(long long a0);
  virtual ~Fuzz879_C4();
};
namespace NsInteropTemplate {
namespace NsSub_ {
  struct TemplateClass {
    template <typename U, int N0> void mt_func(U a0);
  };
  template <typename U, int N0> void tfn_func(U a0);

  // Explicit specializations
  template <> void TemplateClass::mt_func<double, 42>(double a0);
  template <> void tfn_func<double, 42>(double a0);
}
}

void check_template_interop(NsInteropTemplate::NsSub_::TemplateClass *p);

namespace NsInteropVirtOverride {
  struct Base {
    virtual void print_class();
    virtual int print_class_val();
    virtual ~Base();
    Base();
  };

  struct CDerived : public Base {
    virtual void print_class();
    virtual int print_class_val();
    virtual ~CDerived();
    CDerived();
  };

  struct VDerived : public virtual CDerived {
    virtual void print_class();
    virtual int print_class_val();
    virtual ~VDerived();
    VDerived();
  };

  struct CpDerived : public VDerived {
    virtual void print_class();
    virtual int print_class_val();
    virtual ~CpDerived();
    CpDerived();
  };

  int check_virt_override_interop();
}

#endif
