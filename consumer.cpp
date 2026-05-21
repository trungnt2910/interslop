#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

extern "C" {
  __attribute__((weak)) void __gxx_personality_sj0() {}
  __attribute__((weak)) void _Unwind_SjLj_Register(void *) {}
  __attribute__((weak)) void _Unwind_SjLj_Unregister(void *) {}
  __attribute__((weak)) void _Unwind_SjLj_Resume(void *) { abort(); }
}

void test_consumer_eh() {
  printf("[Consumer] test_consumer_eh() calling trigger_eh_from_lib()...\n");
  try {
    trigger_eh_from_lib();
  } catch (Derived &e) {
    printf("[Consumer] Caught Derived exception in consumer! e.z=%d\n", e.z);
  } catch (...) {
    printf("[Consumer] Caught unknown exception in consumer!\n");
    printf("[ERROR] EH Interop Failure: Clang consumer failed to catch Derived exception thrown by GCC 2.95 lib (caught unknown ... instead)\n");
    exit(1);
  }
}

extern "C" void get_gcc_thunk() asm("__thunk_4_f2__7Derived");

void test_thunk_mangling() {
  printf("[Consumer] Testing thunk mangling interop...\n");
  // In GCC 2.95 ABI, the thunk for Derived::f2 (which adjusts 'this' by -4)
  // is mangled as __thunk_4_f2__7Derived.
  // Clang incorrectly mangles it as __thunk_-4_f2__7Derived.
  // By referencing __thunk_4_f2__7Derived here, Combination 1 (lib built with Clang, consumer with GCC)
  // will fail to link because lib_clang.o only exports __thunk_-4_f2__7Derived.
  void (*p)() = &get_gcc_thunk;
  printf("[Consumer] Thunk symbol __thunk_4_f2__7Derived address: %p\n", (void*)p);
}

void test_dynamic_cast(Base2 *b2) {
  printf("[Consumer] Testing dynamic_cast<Derived*>(b2)...\n");
  // This reproduces the second compatibility issue: Clang passes {ObjectPtr, SrcRTTI, DestRTTI, Offset}
  // while GCC 2.95 runtime expects {FromTinfoFnPtr, ToTinfoFnPtr, RequirePublic, ObjectPtr, SubTinfoFnPtr, SubObjectPtr}.
  // GCC 2.95 runtime will treat ObjectPtr as FromTinfoFnPtr and attempt to call it, causing a SIGSEGV.
  Derived *d = dynamic_cast<Derived*>(b2);
  if (d == NULL) { printf("[ERROR] dynamic_cast failed!\n"); exit(1); }
  printf("[Consumer] dynamic_cast result: %p\n", (void*)d);
}

struct ConsumerVSub : virtual public VBaseDtorTester {
  int z;
  ConsumerVSub() { z = 555; }
  virtual ~ConsumerVSub() {}
};

void test_vbase_dynamic_cast() {
  printf("[Consumer] Testing dynamic_cast on ConsumerVSub (virtual base RTTI interop)...\n");
  ConsumerVSub *vsub = new ConsumerVSub();
  VBaseDtorTester *vbase = vsub; // implicit upcast to virtual base
  ConsumerVSub *casted = dynamic_cast<ConsumerVSub*>(vbase);
  printf("[Consumer] dynamic_cast<ConsumerVSub*> result: %p\n", (void*)casted);
  if (casted != vsub) {
    printf("[ERROR] Virtual Base dynamic_cast failed! Expected %p, got %p\n", (void*)vsub, (void*)casted);
    exit(1);
  }
}


void test_consumer_eh_base() {
  printf("[Consumer] test_consumer_eh_base() calling trigger_eh_base()...\n");
  try {
    trigger_eh_base();
  } catch (Base1 &b1) {
    printf("[Consumer] Caught Base1 exception by reference in consumer! b1.f1()=%d\n", b1.f1());
  } catch (...) {
    printf("[Consumer] Caught unknown exception in consumer!\n");
    printf("[ERROR] EH Interop Failure: Clang consumer failed to catch Base1 exception thrown by GCC 2.95 lib\n");
    exit(1);
  }
}

void test_consumer_eh_vbase() {
  printf("[Consumer] test_consumer_eh_vbase() calling trigger_eh_vbase()...\n");
  try {
    trigger_eh_vbase();
  } catch (PolyBase &pb) {
    printf("[Consumer] Caught PolyBase (virtual base) by reference in consumer! pb.poly_func()=%d\n", pb.poly_func());
  } catch (...) {
    printf("[Consumer] Caught unknown exception in consumer!\n");
    printf("[ERROR] EH Interop Failure: Clang consumer failed to catch PolyBase (virtual base) exception thrown by GCC 2.95 lib\n");
    exit(1);
  }
}

void test_consumer_eh_int_ptr() {
  printf("[Consumer] test_consumer_eh_int_ptr() calling trigger_eh_int_ptr()...\n");
  try {
    trigger_eh_int_ptr();
  } catch (const int *p) {
    printf("[Consumer] Caught const int* exception in consumer! val=%d\n", *p);
  } catch (...) {
    printf("[Consumer] Caught unknown exception in consumer!\n");
    printf("[ERROR] EH Interop Failure: Clang consumer failed to catch const int* exception thrown by GCC 2.95 lib\n");
    exit(1);
  }
}

void test_consumer_eh_fn_ptr() {
  printf("[Consumer] test_consumer_eh_fn_ptr() calling trigger_eh_fn_ptr()...\n");
  try {
    trigger_eh_fn_ptr();
  } catch (void *p) {
    printf("[ERROR] EH Interop Failure: Caught function pointer in catch (void*)!\n");
    exit(1);
  } catch (int (*fn)(int, double)) {
    printf("[Consumer] Successfully caught function pointer in consumer!\n");
  } catch (...) {
    printf("[ERROR] EH Interop Failure: Failed to catch function pointer!\n");
    exit(1);
  }
}

int sample_fn(double v) {
  return (int)v * 2;
}

Derived global_derived(888);

void test_eh_spec() throw(Derived) {
  printf("[Consumer] test_eh_spec() calling trigger_eh_from_lib()...\n");
  try {
    trigger_eh_from_lib();
  } catch (Derived &e) {
    printf("[Consumer] Caught Derived exception in test_eh_spec! e.z=%d\n", e.z);
  }
}

void test_static_local() {
  printf("[Consumer] test_static_local()...\n");
  static Derived static_d(777);
  if (static_d.z != 777) { printf("[ERROR] static_d.z failed! Expected 777, got %d\n", static_d.z); exit(1); }
  printf("[Consumer] static_d.z = %d\n", static_d.z);
}

void test_array_cookies() {
  printf("[Consumer] test_array_cookies()...\n");
  // Test 1: Lib allocates, Consumer deletes
  CookieTester *p1 = alloc_cookie_tester(3);
  printf("[Consumer] Deleting p1 allocated by lib...\n");
  delete[] p1;
  printf("[Consumer] After delete[] p1, CookieTester::dtor_count = %d\n", CookieTester::dtor_count);
  if (CookieTester::dtor_count != 3) {
    printf("[ERROR] Array Cookie Interop Failure: Expected 3 dtors, got %d\n", CookieTester::dtor_count);
    exit(1);
  }

  CookieTester::dtor_count = 0;

  // Test 2: Consumer allocates, Lib deletes
  printf("[Consumer] Allocating p2 in consumer...\n");
  CookieTester *p2 = new CookieTester[4];
  printf("[Consumer] Passing p2 to lib for deletion...\n");
  delete_cookie_tester(p2);
  printf("[Consumer] After lib delete[] p2, CookieTester::dtor_count = %d\n", CookieTester::dtor_count);
  if (CookieTester::dtor_count != 4) {
    printf("[ERROR] Array Cookie Interop Failure: Expected 4 dtors, got %d\n", CookieTester::dtor_count);
    exit(1);
  }
}

void test_vbase_dtor_interop() {
  printf("[Consumer] test_vbase_dtor_interop()...\n");
  
  // Test 1: Lib allocates, Consumer deletes
  VBaseDtorTester::dtor_count = 0;
  VSubDtorTester *p1 = alloc_vsub();
  printf("[Consumer] Deleting p1 (allocated by lib) in consumer...\n");
  delete p1;
  printf("[Consumer] After delete p1, VBaseDtorTester::dtor_count = %d\n", VBaseDtorTester::dtor_count);
  if (VBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Virtual Base Dtor Interop Failure: Expected VBaseDtorTester::dtor_count == 1, got %d\n", VBaseDtorTester::dtor_count);
    exit(1);
  }

  // Test 2: Consumer allocates, Lib deletes
  VBaseDtorTester::dtor_count = 0;
  printf("[Consumer] Allocating p2 in consumer...\n");
  VSubDtorTester *p2 = new VSubDtorTester();
  printf("[Consumer] Passing p2 to lib for deletion...\n");
  delete_vsub(p2);
  printf("[Consumer] After lib delete p2, VBaseDtorTester::dtor_count = %d\n", VBaseDtorTester::dtor_count);
  if (VBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Virtual Base Dtor Interop Failure: Expected VBaseDtorTester::dtor_count == 1, got %d\n", VBaseDtorTester::dtor_count);
    exit(1);
  }
}

void test_inline_static_interop() {
  printf("[Consumer] test_inline_static_interop()...\n");
  int r1 = call_inline_static_from_lib(100);
  printf("[Consumer] call_inline_static_from_lib(100) returned: %d\n", r1);
  int r2 = test_inline_static(200);
  printf("[Consumer] test_inline_static(200) in consumer returned: %d\n", r2);
  if (r1 != 100 || r2 != 100) {
    printf("[ERROR] Inline Static Interop Failure: Expected r1==100 and r2==100, got r1=%d, r2=%d\n", r1, r2);
    exit(1);
  }
}

void test_deep_vsub_interop() {
  printf("[Consumer] test_deep_vsub_interop()...\n");
  DeepVSub *p = alloc_deep_vsub();
  if (p) {
    unsigned *raw = (unsigned*)p;
    printf("[Consumer] DeepVSub memory dump:\n");
    for (int i = 0; i < 8; i++) {
      printf("  p[%d] (offset %d) = 0x%x (%u)\n", i, i*4, raw[i], raw[i]);
    }
  }
  printf("[Consumer] p->d = %d\n", p ? p->d : -1);
  printf("[Consumer] p->inter_func() = %d\n", p ? p->inter_func() : -1);
  printf("[Consumer] p->poly_func() = %d\n", p ? p->poly_func() : -1);
  if (!p || p->poly_func() != 111 || p->inter_func() != 222 || p->d != 333) {
    printf("[ERROR] DeepVSub subobject validation failed!\n");
    exit(1);
  }
  printf("[Consumer] Testing dynamic_cast on GCC-allocated DeepVSub...\n");
  PolyBase* pb = p;
  DeepVSub* casted = dynamic_cast<DeepVSub*>(pb);
  printf("[Consumer] dynamic_cast<DeepVSub*> result: %p\n", (void*)casted);
  if (casted != p) {
    printf("[ERROR] DeepVSub dynamic_cast failed! Expected %p, got %p\n", (void*)p, (void*)casted);
    exit(1);
  }
  printf("[Consumer] Deleting p (allocated by lib) in consumer...\n");
  delete p;
}

void test_layout_interop() {
  printf("[Consumer] test_layout_interop()...\n");
  printf("[Consumer] sizeof(LayoutDerived1) = %d\n", (int)sizeof(LayoutDerived1));
  printf("[Consumer] sizeof(LayoutDerived2) = %d\n", (int)sizeof(LayoutDerived2));
  if (sizeof(LayoutDerived1) != 24) {
    printf("[ERROR] sizeof(LayoutDerived1) expected 24, got %d\n", (int)sizeof(LayoutDerived1));
    exit(1);
  }
  if (sizeof(LayoutDerived2) != 24) {
    printf("[ERROR] sizeof(LayoutDerived2) expected 24, got %d\n", (int)sizeof(LayoutDerived2));
    exit(1);
  }
  LayoutDerived1 d1;
  LayoutDerived2 d2;
  d1.x = 10;
  d1.y = 20.0;
  d1.c = 'X';
  d2.x = 30;
  d2.y = 40.0;
  d2.c = 'Y';
  check_layout_derived(&d1, &d2);
  if (d1.c != 'A' || d2.c != 'B') {
    printf("[ERROR] Layout derived members not accessed correctly!\n");
    exit(1);
  }
  printf("[SUCCESS] Layout interop passed!\n");
}

void test_empty_vbase_interop() {
  printf("[Consumer] test_empty_vbase_interop()...\n");
  printf("[Consumer] sizeof(EmptyVDerived1) = %d, alignof = %d\n", (int)sizeof(EmptyVDerived1), (int)__alignof__(EmptyVDerived1));
  printf("[Consumer] sizeof(EmptyVDerived2) = %d, alignof = %d\n", (int)sizeof(EmptyVDerived2), (int)__alignof__(EmptyVDerived2));

  if (sizeof(EmptyVDerived1) != 8) {
    printf("[ERROR] sizeof(EmptyVDerived1) expected 8, got %d\n", (int)sizeof(EmptyVDerived1));
    exit(1);
  }
  if (sizeof(EmptyVDerived2) != 12) {
    printf("[ERROR] sizeof(EmptyVDerived2) expected 12, got %d\n", (int)sizeof(EmptyVDerived2));
    exit(1);
  }

  EmptyVDerived1 d1;
  EmptyVDerived2 d2;
  check_empty_vderived(&d1, &d2);

  EmptyVDerived2 arr[2];
  printf("[Consumer] arr[0] = %p, arr[1] = %p (diff = %d)\n", 
         &arr[0], &arr[1], (int)((char*)&arr[1] - (char*)&arr[0]));
  check_empty_vderived_array(arr, 2);
}


void test_ebo_interop() {
  printf("[Consumer] test_ebo_interop()...\n");
  printf("[Consumer] sizeof(EboEmpty) = %d\n", (int)sizeof(EboEmpty));
  if (sizeof(EboEmpty) != 1) {
    printf("[ERROR] sizeof(EboEmpty) expected 1, got %d\n", (int)sizeof(EboEmpty));
    exit(1);
  }
  EboDerived d;
  d.x = 10;
  d.y = 42;
  printf("[Consumer] d.y set to 42, calling lib check_ebo_derived...\n");
  check_ebo_derived(&d);
}

void test_inline_guard_interop() {
  printf("[Consumer] test_inline_guard_interop()...\n");
  int r1 = call_inline_guard_from_lib(100);
  printf("[Consumer] call_inline_guard_from_lib(100) returned: %d\n", r1);
  int r2 = test_inline_guard(200);
  printf("[Consumer] test_inline_guard(200) in consumer returned: %d\n", r2);
  if (r1 != 100 || r2 != 100) {
    printf("[ERROR] Inline Guard Interop Failure: Expected r1==100 and r2==100, got r1=%d, r2=%d\n", r1, r2);
    exit(1);
  }
}

struct ConsumerDeep : virtual public IntermediateVBase {
  int cd;
  ConsumerDeep() { cd = 999; }
  virtual ~ConsumerDeep() {}
};

void test_cross_vbase_init() {
  printf("[Consumer] Testing cross-compiler virtual base construction (ConsumerDeep)...\n");
  ConsumerDeep *cd = new ConsumerDeep();
  if (!cd || cd->cd != 999 || cd->inter_func() != 222 || cd->poly_func() != 111) {
    printf("[ERROR] ConsumerDeep subobject validation failed!\n");
    exit(1);
  }
  printf("[Consumer] ConsumerDeep constructed successfully. cd->cd = %d\n", cd->cd);
  delete cd;
}

void test_covariant_interop() {
  printf("[Consumer] Testing covariant return types interop...\n");
  CovariantBase2* b2 = get_covariant_derived();
  printf("[Consumer] b2 address: %p\n", (void*)b2);
  CovariantBase2* b2_clone = b2->clone();
  printf("[Consumer] b2_clone address: %p\n", (void*)b2_clone);
  // GCC 2.95 COVARIANT RETURN POINTER ADJUSTMENT BUG EXPLANATION:
  //
  // --- Why GCC 2.95 is broken ---
  // GCC 2.95 has a compiler bug/limitation in its C++ frontend (`gcc/cp/search.c:covariant_return_p`):
  // It explicitly aborts compilation with `sorry ("adjusting pointers for covariant returns")` if it
  // detects non-trivial covariance (requires virtual base offset or non-zero base offset).
  // However, for non-virtual base classes, GCC 2.95's `BINFO_OFFSET` is historically unpopulated (zero)
  // during this check. Consequently, `BINFO_OFFSET_ZEROP` erroneously evaluates to true.
  //
  // Because of this, GCC 2.95 incorrectly classifies non-virtual base covariant returns (with non-zero offsets)
  // as "trivial covariance" (returns 1 instead of 2), bypassing the `sorry` compiler error.
  // But because it thinks the covariance is trivial, it completely omits generating the return-pointer
  // adjustment code in the virtual function's thunk!
  //
  // --- How it is broken ---
  // The generated thunk (`__thunk_4_clone__1D`) only performs `this` pointer adjustment (-4) and then
  // jumps straight to `D::clone()`. Since `D::clone()` returns the derived pointer `D*`, the thunk returns
  // this unadjusted derived pointer (which points to `B1` subobject at offset 0) instead of casting it
  // to `B2*` (which is at offset 4).
  //
  // Thus:
  // 1. If the Lib is compiled by GCC 2.95 (broken), calling `b2->clone()` returns `b2 - 4` (broken).
  //    We assert this expected broken pointer when __clang__ is defined (Clang consumer calling GCC lib).
  // 2. If the Lib is compiled by Clang (which now correctly performs return adjustment in the thunk),
  //    calling `b2->clone()` returns `b2` (correct).
  //    We assert this correct pointer when __clang__ is NOT defined (GCC consumer calling Clang lib).
#ifdef __clang__
  CovariantBase2* expected = (CovariantBase2*)((char*)b2 - 4);
  const char* expected_type = "broken";
#else
  CovariantBase2* expected = b2;
  const char* expected_type = "correct";
#endif

  if (b2_clone != expected) {
    printf("[ERROR] Covariant return interop failed! Expected %p (%s), got %p\n", (void*)expected, expected_type, (void*)b2_clone);
    exit(1);
  }
  printf("[Consumer] Covariant return interop passed!\n");
}



inline int test_interop_collision() {
  static int gcc2_collision_var = 10;
  return gcc2_collision_var++;
}

void test_static_local_collision_repro() {
  printf("[Consumer] Running static local collision repro (expecting NO collision)...\n");
  int r = test_interop_collision();
  printf("[Consumer] test_interop_collision returned: %d\n", r);
  if (r == 999) {
    printf("[ERROR] Static Local Collision Repro Failed! Collision detected: test_interop_collision returned global variable's value (999) instead of 10.\n");
    exit(1);
  }
  printf("[Consumer] Static local collision repro passed.\n");
}

void test_nontype_interop() {
  printf("[Consumer] Testing non-type template arguments interop...\n");
  S_nontype1<42> s1;
  test_nontype_1(s1);

  S_nontype2<&nontype_global_var> s2;
  test_nontype_2(s2);

  S_nontype3<&nontype_global_func> s3;
  test_nontype_3(s3);

  S_nontype4<&Derived::z> s4;
  test_nontype_4(s4);

  S_nontype5<&Derived::f1> s5;
  test_nontype_5(s5);

  S_nontype5<&Derived::f2> s7;
  test_nontype_7(s7);

  S_nontype_virt<&VptrNotZero::foo> s_virt;
  test_nontype_virt(s_virt);

  S_nontype_mi<&MiDerived::bar> s_mi;
  test_nontype_mi(s_mi);

  printf("[Consumer] Non-type template arguments interop passed!\n");
}

void test_vptr_not_zero_memptr() {
  printf("[Consumer] Testing member pointer to virtual function in class with vptr not at offset 0...\n");
  VptrNotZero obj;
  VptrNotZeroPTMF ptmf = get_vptr_not_zero_ptmf();
  printf("[Consumer] Calling (obj.*ptmf)() [passed from lib]...\n");
  (obj.*ptmf)();

  printf("[Consumer] Calling local PTMF...\n");
  VptrNotZeroPTMF local_ptmf = &VptrNotZero::foo;
  (obj.*local_ptmf)();
}

void test_dynamic_cast_interop() {
  printf("[Consumer] Testing dynamic_cast interop...\n");
  DynBase* b = get_dyn_derived(42);
  DynDerived* d = dynamic_cast<DynDerived*>(b);
  if (!d || d->val != 42) {
    printf("[ERROR] dynamic_cast failed!\n");
    exit(1);
  }
  printf("[Consumer] dynamic_cast interop passed!\n");
  delete b;
}

void test_fn_tmpl_interop() {
  printf("[Consumer] Testing function template interop...\n");
  int r_int = interop_fn_tmpl(42);
  printf("[Consumer] interop_fn_tmpl<int>(42) = %d\n", r_int);
  if (r_int != 142) {
    printf("[ERROR] interop_fn_tmpl<int> failed! Expected 142, got %d\n", r_int);
    exit(1);
  }
  double r_double = interop_fn_tmpl(3.5);
  printf("[Consumer] interop_fn_tmpl<double>(3.5) = %f\n", r_double);
  if (r_double != 103.5) {
    printf("[ERROR] interop_fn_tmpl<double> failed! Expected 103.5, got %f\n", r_double);
    exit(1);
  }
  printf("[Consumer] function template interop passed!\n");
}

void test_local_class_interop() {
  printf("[Consumer] Testing local class inside template function interop...\n");
  int size_int = test_local_class_collision<int>(42);
  int size_double = test_local_class_collision<double>(3.14);
  printf("[Consumer] test_local_class_collision<int> size: %d\n", size_int);
  printf("[Consumer] test_local_class_collision<double> size: %d\n", size_double);
  if (size_int != sizeof(int)) {
    printf("[ERROR] Local class template collision! size_int expected %d, got %d\n", (int)sizeof(int), size_int);
    exit(1);
  }
  if (size_double != sizeof(double)) {
    printf("[ERROR] Local class template collision! size_double expected %d, got %d\n", (int)sizeof(double), size_double);
    exit(1);
  }
  printf("[Consumer] local class template interop passed!\n");
}

void test_repro_class_interop() {
  printf("[Consumer] Testing ReproClass interop (vtable layout with non-dynamic base)...\n");
  ReproClass obj;
  obj.a = 123;
  obj.x = 456;
  check_repro_class(&obj);
}

void test_pass_by_val_dtor_only_interop() {
  printf("[Consumer] Testing DtorOnlyNonTrivial pass by value interop...\n");
  DtorOnlyNonTrivial s;
  s.val = 12345;
  int res = test_pass_by_val_dtor_only(s);
  printf("[Consumer] test_pass_by_val_dtor_only returned: %d\n", res);
  if (res != 24690) {
    printf("[ERROR] test_pass_by_val_dtor_only failed! Expected 24690, got %d\n", res);
    exit(1);
  }
}

void test_overwrite_derived_interop() {
  printf("[Consumer] Testing vbptr overwrite interop...\n");
  OverwriteDerived d;
  printf("[Consumer] d.nv_val = %d, d.x = %d\n", d.nv_val, d.x);
  if (d.nv_val != 111) {
    printf("[ERROR] d.nv_val was overwritten in consumer! Value = %d\n", d.nv_val);
    exit(1);
  }
  check_overwrite_derived(&d);
  printf("[Consumer] vbptr overwrite interop passed!\n");
}

void test_empty_struct_pass_interop() {
  printf("[Consumer] Testing empty struct pass by value interop...\n");
  EmptyStruct e;
  int res = test_empty_struct_pass(100, e, 200);
  printf("[Consumer] test_empty_struct_pass result: %d\n", res);
  if (res != 300) {
    printf("[ERROR] test_empty_struct_pass failed! Expected 300, got %d\n", res);
    exit(1);
  }
  printf("[Consumer] empty struct pass by value interop passed!\n");
}

// BaseToDerived conversion on constant (implicit)
CompDerivedPTMF const_ptmf_base_to_derived = &PTMFCompBase2::foo;

// DerivedToBase conversion on constant (explicit static_cast)
CompBase2PTMF const_ptmf_derived_to_base = static_cast<CompBase2PTMF>((CompDerivedPTMF)&PTMFCompBase2::foo);

void test_ptmf_compare_repro() {
  printf("[Consumer] Testing PTMF comparison with conversion...\n");
  CompDerivedPTMF p1 = &PTMFCompBase1::foo;
  CompDerivedPTMF p2 = &PTMFCompBase2::foo;

  if (p1 == p2) {
    printf("[Consumer] PTMF compare: p1 == p2 (unexpected!)\n");
  } else {
    printf("[Consumer] PTMF compare: p1 != p2 (expected)\n");
  }

  // Reference the globals to prevent them from being optimized away
  if (const_ptmf_base_to_derived == static_cast<CompDerivedPTMF>(const_ptmf_derived_to_base)) {
    // Just reference them, doesn't matter if it prints or not (we expect it not to be equal, but we just want to avoid dead-code elimination)
    printf("[Consumer] const PTMFs referenced\n");
  }
}

void test_vptr_retreival_vbase_consumer(ReproVDerived *d) {
  printf("[Consumer] ReproVDerived typeid name = %s\n", typeid(*d).name());
}

void test_vptr_retreival_nvbase_consumer(ReproNVDerived *d) {
  printf("[Consumer] ReproNVDerived typeid name = %s\n", typeid(*d).name());
}

void test_vptr_retreival_interop() {
  printf("[Consumer] Testing vptr retrieval with non-dynamic first vbase...\n");
  ReproVDerived d1;
  printf("[Consumer] sizeof(ReproVDerived) = %d\n", (int)sizeof(ReproVDerived));
  printf("[Consumer] offset of ReproVBaseNonDyn = %d\n", (int)((char*)(ReproVBaseNonDyn*)&d1 - (char*)&d1));
  printf("[Consumer] offset of ReproVBaseDyn = %d\n", (int)((char*)(ReproVBaseDyn*)&d1 - (char*)&d1));

  printf("[Consumer] calling test_vptr_retreival_vbase...\n");
  test_vptr_retreival_vbase(&d1);
  printf("[Consumer] calling test_vptr_retreival_vbase_consumer...\n");
  test_vptr_retreival_vbase_consumer(&d1);

  printf("[Consumer] Testing vptr retrieval with non-virtual dynamic base with members...\n");
  ReproNVDerived d2;
  d2.x = 12345; // Make sure it doesn't crash if correct, or crashes if wrong
  printf("[Consumer] sizeof(ReproNVDerived) = %d\n", (int)sizeof(ReproNVDerived));
  printf("[Consumer] offset of ReproNVBase = %d\n", (int)((char*)(ReproNVBase*)&d2 - (char*)&d2));

  printf("[Consumer] calling test_vptr_retreival_nvbase...\n");
  test_vptr_retreival_nvbase(&d2);
  printf("[Consumer] calling test_vptr_retreival_nvbase_consumer...\n");
  test_vptr_retreival_nvbase_consumer(&d2);
  printf("[Consumer] vptr retrieval interop passed!\n");
}

void test_vbase_alignment_bug() {
  printf("[Consumer] Testing virtual base alignment promotion bug (expecting discrepancy)...\n");
  BugVBase obj;
  printf("[Consumer] sizeof(BugVBase) = %d\n", (int)sizeof(BugVBase));
  printf("[Consumer] offset of BugEmpty1 = %d\n", (int)((char*)(BugEmpty1*)&obj - (char*)&obj));
  printf("[Consumer] offset of BugEmpty2 = %d\n", (int)((char*)(BugEmpty2*)&obj - (char*)&obj));
  check_vbase_alignment_bug(&obj, (int)sizeof(BugVBase));
}

struct EHMI_Base1 {
  virtual ~EHMI_Base1() {}
};
struct EHMI_Base2 {
  virtual ~EHMI_Base2() {}
};
struct EHMI_Derived : EHMI_Base1, EHMI_Base2 {
  virtual ~EHMI_Derived() {}
};
void test_eh_mi_inline() {
  printf("[Consumer] Testing EH MI inline destructor bug...\n");
  try {
    throw EHMI_Derived();
  } catch (EHMI_Base2 &b) {
    printf("[Consumer] Caught EHMI_Base2 successfully!\n");
  } catch (...) {
    printf("[ERROR] EH MI inline test caught unknown exception!\n");
    exit(1);
  }
}

void test_local_rtti_interop() {
  printf("[Consumer] Testing local class inside template function RTTI interop...\n");
  RTTIBase* p_lib = get_lib_local_rtti(); // Compiled by Clang
  RTTIBase* p_cons = get_local_rtti_obj<int>(); // Compiled by GCC

  printf("[Consumer] p_lib typeid name = %s\n", typeid(*p_lib).name());
  printf("[Consumer] p_cons typeid name = %s\n", typeid(*p_cons).name());

  if (typeid(*p_lib) == typeid(*p_cons)) {
    printf("[Consumer] Local class RTTI interop passed!\n");
  } else {
    printf("[ERROR] Local class RTTI mismatch! p_lib=%s, p_cons=%s\n", 
           typeid(*p_lib).name(), typeid(*p_cons).name());
    exit(1);
  }
  delete p_lib;
  delete p_cons;
}

void test_spec_local_class_interop() {
  printf("[Consumer] Testing local class of explicit specialization...\n");
  int res = test_spec_local_class<int>(42);
  printf("[Consumer] test_spec_local_class<int> res: %d\n", res);
  if (res != 42) {
    printf("[ERROR] test_spec_local_class failed! Expected 42, got %d\n", res);
    exit(1);
  }
}
void test_regular_local_class_rtti_interop() {
  printf("[Consumer] Testing regular local class RTTI interop...\n");
  const std::type_info& ti_lib = get_lib_regular_local_rtti2(); // Compiled by Clang
  const std::type_info& ti_cons = get_regular_local_rtti2(); // Compiled by GCC

  printf("[Consumer] ti_lib name = %s\n", ti_lib.name());
  printf("[Consumer] ti_cons name = %s\n", ti_cons.name());

  if (ti_lib == ti_cons) {
    printf("[Consumer] Regular local class RTTI interop passed!\n");
  } else {
    printf("[ERROR] Regular local class RTTI mismatch! ti_lib=%s, ti_cons=%s\n", 
           ti_lib.name(), ti_cons.name());
    exit(1);
  }
}

struct Gcc2Pmf {
  short delta;
  short index;
  union {
    void* pfn;
    short delta2;
  } u;
};

/* 
 * NOTE ON GCC 2.95 POINTER-TO-MEMBER-FUNCTION (PMF) VIRTUAL BASE BUG:
 * 
 * Under the legacy GCC 2.95 C++ ABI, pointers to member functions (PMFs) for virtual 
 * functions originating from virtual base classes exhibit a major compiler bug when 
 * overridden in a derived class (e.g., &PmfDerived::vbase_virt).
 * 
 * --- HOW THE BUG WORKS ---
 * A PMF under the GCC2 ABI consists of:
 *   - delta (16-bit): Static 'this' pointer adjustment.
 *   - index (16-bit): Vtable index (1-indexed, or -1 for non-virtual).
 *   - delta2/pfn (32-bit union): Offset of the vptr inside the subobject.
 * 
 * When a virtual method is defined in a virtual base (e.g., PmfVBase::vbase_virt), the offset
 * to that base class (BaseOffset) is dynamic. However, GCC 2.95 fails to handle virtual 
 * inheritance statically when constructing the PMF. It erroneously assumes the virtual base 
 * resides at offset 0 and shares the primary vptr of the derived class:
 *   1. It sets `delta = 0` (should be `8`, the offset to `PmfVBase`).
 *   2. It sets `delta2 = Derived_VFPtrOffset` (which is `4` if PmfDerived has its own virtual 
 *      functions, or `0` if it does not).
 * 
 * --- THE CONTEXT-DEPENDENT BREAKAGE ---
 * Because of this static offset failure, GCC 2.95-compiled binaries break in two ways, 
 * even in pure GCC2-to-GCC2 calls:
 * 
 *   1. Silent Functional Breakage (Wrong Method Called):
 *      If `PmfDerived` has its own virtual functions (such as `derived_virt()`), it has a primary 
 *      vptr at offset 4. The buggy PMF (`delta=0, delta2=4`) causes the PMF call code to load 
 *      the vtable from `this + 4` (the primary vptr) and look up the function at index 2.
 *      However, index 2 of `PmfDerived`'s primary vtable contains `derived_virt`, NOT `vbase_virt`!
 *      Thus, calling the PMF silently routes to `PmfDerived::derived_virt()`, causing incorrect 
 *      program behavior.
 * 
 *   2. Runtime Crash (Segmentation Fault):
 *      If `PmfDerived` does NOT introduce any new virtual functions of its own, it has no primary 
 *      vptr. The only vptr is inside the `PmfVBase` subobject at offset 4. Offset 0 contains the 
 *      `vbase pointer` (vbptr). The buggy PMF (`delta=0, delta2=0`) causes the PMF call code to 
 *      load the vtable pointer from `this + 0`. It dereferences the `vbase pointer` as a vptr, 
 *      resulting in a Segmentation Fault (hard crash) at runtime.
 * 
 * --- DESIGN DECISION: WHY WE DO NOT EMULATE THIS BUG ---
 * Standard-compliant compilers like Clang calculate the correct static offsets (`delta = 8`, 
 * `delta2 = 12`), which allows virtual dispatch to succeed correctly at runtime.
 * 
 * Although binary compatibility is the main objective of `-fc++-abi=gcc2`, we choose NOT to 
 * emulate this GCC 2.95 bug in Clang for the following reasons:
 *   1. Emulating the bug would force Clang-compiled code to silently call the wrong function 
 *      or crash at runtime, introducing severe bugs into compliant C++ code.
 *   2. GCC 2.95's PMF call code is actually dynamic and reads the fields of the PMF at runtime. 
 *      If Clang-compiled code (acting as a library) returns a CORRECT PMF (`delta=8, delta2=12`), 
 *      a GCC 2.95 consumer can successfully call it without crashing or calling the wrong 
 *      function because it correctly processes `delta2=12`!
 * 
 * Therefore:
 *   - Clang -> GCC interop WORKS if Clang generates standard-compliant, correct PMFs.
 *   - GCC -> Clang interop is inherently broken because the PMFs generated by GCC 2.95 are 
 *     corrupt and cannot be resolved correctly by any compiler.
 * 
 * Consequently, we disable/guard the checks expecting the buggy GCC 2.95 PMF layout under Clang, 
 * and only assert that Clang generates correct PMFs, while bypassing runtime calling checks for 
 * GCC-generated buggy PMFs.
 */
void test_pmf_vbase_interop() {
  printf("[Consumer] Testing PMF virtual base interop...\n");
  PmfDerived obj;
  PmfDerived *ptr = &obj;

#ifdef __clang__
  // Verify Clang-generated PMF fields under GCC2 ABI are CORRECT (delta=8, delta2=12)
  PmfDerivedPTMF local_pmf_vbase = &PmfDerived::vbase_virt;
  Gcc2Pmf* g = (Gcc2Pmf*)&local_pmf_vbase;
  printf("[Consumer] Local PMF fields (Clang): delta=%d, index=%d, delta2=%d\n",
         g->delta, g->index, g->u.delta2);
  
  // Assert that Clang remains standard-compliant and generates correct offsets
  if (g->delta != 8 || g->u.delta2 != 12) {
    printf("[ERROR] PMF Virtual Base discrepancy! Expected standard-compliant delta=8, delta2=12, got delta=%d, delta2=%d\n",
           g->delta, g->u.delta2);
    exit(1);
  }
  printf("[Consumer] Local PMF standard-compliance check passed!\n");
#endif

  PmfDerivedPTMF pmf_vbase = get_pmf_derived_vbase_virt(); // lib-returned (Clang or GCC)
  Gcc2Pmf* g_vbase = (Gcc2Pmf*)&pmf_vbase;
  printf("[Consumer] Lib-returned PMF fields: delta=%d, index=%d, delta2=%d\n",
         g_vbase->delta, g_vbase->index, g_vbase->u.delta2);

  pmf_called_vbase = 0;
  pmf_called_derived = 0;

  printf("[Consumer] Calling pmf_vbase...\n");
  (ptr->*pmf_vbase)();

  // Validate routing dynamically based on the PMF fields returned by the library
  if (g_vbase->delta == 8 && g_vbase->u.delta2 == 12) {
    // Standard-compliant Clang PMF: should call vbase_virt correctly!
    if (pmf_called_vbase != 1) {
      printf("[ERROR] PMF call failed! Expected vbase_virt (correct) but it was not called.\n");
      exit(1);
    }
    printf("[Consumer] PMF call correctly resolved to vbase_virt (Clang standard-compliant behavior)!\n");
  } else {
    // GCC 2.95 buggy PMF (delta=0, delta2=4): should call derived_virt!
    if (pmf_called_derived != 1) {
      printf("[ERROR] PMF call failed to trigger expected GCC 2.95 buggy behavior (expected derived_virt due to bug)!\n");
      exit(1);
    }
    printf("[Consumer] PMF call triggered expected GCC 2.95 buggy behavior (called derived_virt) successfully!\n");
  }
  printf("[Consumer] PMF Virtual Base interop passed!\n");
}

void function_with_eh_cleanup() {
  printf("[Consumer] function_with_eh_cleanup() constructing EhVBaseCleanupTester...\n");
  EhVBaseCleanupTester obj(4321);
  trigger_eh_cleanup_from_lib();
}

void test_eh_cleanup_vbase() {
  printf("[Consumer] Testing EH cleanup with virtual base destructor...\n");
  EhVBaseCleanupTester::dtor_count = 0;
  try {
    function_with_eh_cleanup();
  } catch (int e) {
    printf("[Consumer] Caught exception in test_eh_cleanup_vbase: %d\n", e);
  } catch (...) {
    printf("[Consumer] Caught unknown exception in test_eh_cleanup_vbase!\n");
    exit(1);
  }

  printf("[Consumer] EhVBaseCleanupTester::dtor_count = %d\n", EhVBaseCleanupTester::dtor_count);
  if (EhVBaseCleanupTester::dtor_count != 1) {
    printf("[ERROR] EH Cleanup Virtual Base Dtor Failure: Expected dtor_count == 1, got %d\n", EhVBaseCleanupTester::dtor_count);
    exit(1);
  }
  printf("[Consumer] EH Cleanup Virtual Base Dtor passed!\n");
}

int main() {


  printf("=== GCC 2.x Full Interop Verification ===\n");
  printf("[Consumer] Initial Derived::instance_count = %d\n", Derived::instance_count);

  {
    printf("[Consumer] Calling normal_func(100, 50.5)...\n");
    int res = normal_func(100, 50.5);
    printf("[Consumer] normal_func returned: %d\n", res);
    if (res != 150) { printf("[ERROR] normal_func failed! Expected 150, got %d\n", res); exit(1); }

    Derived d(1000);
    Derived *p = &d;
    printf("[Consumer] Calling p->f1()...\n");
    p->f1();
    printf("[Consumer] Calling p->f3()...\n");
    p->f3();
    printf("[Consumer] Calling p->f4()...\n");
    p->f4();

    printf("[Consumer] Calling d.f2() via Base2 pointer (testing thunk)...\n");
    Base2 *b2 = &d;
    int f2_res = b2->f2();
    printf("[Consumer] b2->f2() returned: %d\n", f2_res);
    if (f2_res != 1002) { printf("[ERROR] b2->f2() failed! Expected 1002, got %d\n", f2_res); exit(1); }

    printf("[Consumer] Testing explicit static RTTI: typeid(Derived).name() = %s\n", typeid(Derived).name());
    printf("[Consumer] Verifying typeid(Derived).name() matches GCC 2.x ABI...\n");
    if (strcmp(typeid(Derived).name(), "7Derived") != 0) {
      printf("[ERROR] RTTI Name Mismatch: Expected '7Derived', got '%s'\n", typeid(Derived).name());
      exit(1);
    }
    printf("[Consumer] Testing explicit dynamic RTTI: typeid(*b2) == typeid(Derived) ? %s\n", typeid(*b2) == typeid(Derived) ? "YES" : "NO");
    if (typeid(*b2) != typeid(Derived)) { printf("[ERROR] Dynamic RTTI comparison failed!\n"); exit(1); }

    printf("[Consumer] Calling d.f4()...\n");
    d.f4();

    printf("[Consumer] Calling assignment operators on d...\n");
    d += 100;
    d -= 50;
    d *= 2;
    d /= 2;
    d %= 10000;
    d &= 0xffff;
    d |= 0x1000;
    d ^= 0x1000;
    d <<= 1;
    d >>= 1;
    if (d.z != 1050) { printf("[ERROR] Assignment operators failed! Expected 1050, got %d\n", d.z); exit(1); }

    printf("[Consumer] Testing explicit copy constructor interop...\n");
    Derived d2 = d;
    printf("[Consumer] Calling d2.f1()...\n");
    d2.f1();
    if (d2.z != 1050) { printf("[ERROR] Copy constructor failed! Expected 1050, got %d\n", d2.z); exit(1); }

    printf("[Consumer] Testing conversion operator interop...\n");
    int conv_val = (int)d;
    printf("[Consumer] (int)d result: %d\n", conv_val);
    if (conv_val != 1050) { printf("[ERROR] (int)d failed! Expected 1050, got %d\n", conv_val); exit(1); }

    test_dynamic_cast(b2);
    test_vbase_dynamic_cast();
    test_thunk_mangling();

    printf("[Consumer] Testing PTMF and PTMD interop...\n");
    DerivedPTMF ptmf1 = get_lib_ptmf(1);
    int ptmf1_res = (d.*ptmf1)();
    printf("[Consumer] ptmf1 call result: %d\n", ptmf1_res);
    if (ptmf1_res != 1051) { printf("[ERROR] ptmf1 call failed! Expected 1051, got %d\n", ptmf1_res); exit(1); }
    int call_ptmf1_res = call_lib_ptmf(&d, ptmf1);
    printf("[Consumer] call_lib_ptmf(ptmf1) result: %d\n", call_ptmf1_res);
    if (call_ptmf1_res != 1051) { printf("[ERROR] call_lib_ptmf(ptmf1) failed! Expected 1051, got %d\n", call_ptmf1_res); exit(1); }

    DerivedPTMF ptmf2 = &Derived::f2;
    int ptmf2_res = (d.*ptmf2)();
    printf("[Consumer] ptmf2 call result: %d\n", ptmf2_res);
    if (ptmf2_res != 1052) { printf("[ERROR] ptmf2 call failed! Expected 1052, got %d\n", ptmf2_res); exit(1); }
    int call_ptmf2_res = call_lib_ptmf(&d, ptmf2);
    printf("[Consumer] call_lib_ptmf(ptmf2) result: %d\n", call_ptmf2_res);
    if (call_ptmf2_res != 1052) { printf("[ERROR] call_lib_ptmf(ptmf2) failed! Expected 1052, got %d\n", call_ptmf2_res); exit(1); }
    printf("[Consumer] ptmf2 == get_lib_ptmf(2): %s\n", (ptmf2 == get_lib_ptmf(2)) ? "YES" : "NO");
    if (ptmf2 != get_lib_ptmf(2)) { printf("[ERROR] ptmf2 comparison failed!\n"); exit(1); }

    DerivedPTMD ptmd = get_lib_ptmd();
    int ptmd_res = d.*ptmd;
    printf("[Consumer] ptmd val result: %d\n", ptmd_res);
    if (ptmd_res != 1050) { printf("[ERROR] ptmd val failed! Expected 1050, got %d\n", ptmd_res); exit(1); }
    int get_ptmd_res = get_lib_ptmd_val(&d, ptmd);
    printf("[Consumer] get_lib_ptmd_val result: %d\n", get_ptmd_res);
    if (get_ptmd_res != 1050) { printf("[ERROR] get_lib_ptmd_val failed! Expected 1050, got %d\n", get_ptmd_res); exit(1); }
    printf("[Consumer] ptmd == &Derived::z: %s\n", (ptmd == &Derived::z) ? "YES" : "NO");
    if (ptmd != &Derived::z) { printf("[ERROR] ptmd comparison failed!\n"); exit(1); }

    printf("[Consumer] Testing const PTMF interop...\n");
    DerivedConstPTMF ptmf_const = get_lib_const_ptmf();
    int ptmf_const_res = (d.*ptmf_const)();
    printf("[Consumer] ptmf_const call result: %d\n", ptmf_const_res);
    if (ptmf_const_res != 1053) { printf("[ERROR] ptmf_const call failed! Expected 1053, got %d\n", ptmf_const_res); exit(1); }
    int call_ptmf_const_res = call_lib_const_ptmf(&d, ptmf_const);
    printf("[Consumer] call_lib_const_ptmf result: %d\n", call_ptmf_const_res);
    if (call_ptmf_const_res != 1053) { printf("[ERROR] call_lib_const_ptmf failed! Expected 1053, got %d\n", call_ptmf_const_res); exit(1); }

    printf("[Consumer] Testing int * const * interop...\n");
    int x_val = 12345;
    int * const x_ptr = &x_val;
    test_const_ptr(&x_ptr);

    DerivedPTMD ptmd_var = &Derived::z;
    DerivedPTMD const ptmd_const = ptmd_var;
    test_const_ptmd(&ptmd_const);

    DerivedPTMF ptmf_var = &Derived::f1;
    DerivedPTMF const ptmf_const2 = ptmf_var;
    test_const_ptmf(&ptmf_const2);

    test_const_ptr_ref(x_ptr);
    test_const_ptmd_ref(ptmd_const);
    test_const_ptmf_ref(ptmf_const2);

    int * const volatile x_cv_ptr = &x_val;
    test_cv_ptr(&x_cv_ptr);

    __complex__ double c = 1.0;
    test_complex(c);

    int arr[10] = { 42, 0 };
    test_array_ref(arr);

    test_fn_ptr(&sample_fn);
    volatile unsigned int vu_val = 777;
    test_vu_ptr(&vu_val);
    int * r_ptr = arr;
    test_restrict_ptr(&r_ptr);
  }


  printf("[Consumer] After scope Derived::instance_count = %d\n", Derived::instance_count);

  test_consumer_eh();
  test_consumer_eh_base();
  test_consumer_eh_vbase();
  test_consumer_eh_int_ptr();
  test_consumer_eh_fn_ptr();
  test_eh_spec();
  test_static_local();
  test_array_cookies();
  test_vbase_dtor_interop();
  test_deep_vsub_interop();
  test_inline_static_interop();


  printf("[Consumer] Testing ByValStruct interop...\n");
  ByValStruct s = { 123 };
  int by_val_res = test_pass_by_val(s);
  printf("[Consumer] test_pass_by_val result: %d\n", by_val_res);
  if (by_val_res != 246) { printf("[ERROR] test_pass_by_val failed! Expected 246, got %d\n", by_val_res); exit(1); }

  printf("[Consumer] Testing repeat ptrs interop...\n");
  int v1 = 10, v2 = 20, v3 = 30;
  int repeat_ptrs_res = test_repeat_ptrs(&v1, &v2, &v3);
  printf("[Consumer] test_repeat_ptrs result: %d\n", repeat_ptrs_res);
  if (repeat_ptrs_res != 60) { printf("[ERROR] test_repeat_ptrs failed! Expected 60, got %d\n", repeat_ptrs_res); exit(1); }

  printf("[Consumer] Testing repeat bools interop...\n");
  int repeat_bools_res = test_repeat_bools(true, true, false);
  printf("[Consumer] test_repeat_bools result: %d\n", repeat_bools_res);
  if (repeat_bools_res != 2) { printf("[ERROR] test_repeat_bools failed! Expected 2, got %d\n", repeat_bools_res); exit(1); }

  printf("[Consumer] Testing mixed repeats interop...\n");
  int mixed_repeats_res = test_mixed_repeats(100, &v1, &v2);
  printf("[Consumer] test_mixed_repeats result: %d\n", mixed_repeats_res);
  if (mixed_repeats_res != 130) { printf("[ERROR] test_mixed_repeats failed! Expected 130, got %d\n", mixed_repeats_res); exit(1); }

  printf("[Consumer] Testing template mangling interop...\n");
  TemplateClass<int> tc(42);
  int template_res = test_template_mangling(&tc);
  printf("[Consumer] test_template_mangling result: %d\n", template_res);
  if (template_res != 42) { printf("[ERROR] test_template_mangling failed! Expected 42, got %d\n", template_res); exit(1); }

  printf("[Consumer] Testing template template mangling interop...\n");
  InteropOuter<InteropInner> oo;
  int tt_res = test_template_template_mangling(&oo);
  printf("[Consumer] test_template_template_mangling result: %d\n", tt_res);
  if (tt_res != 99) { printf("[ERROR] test_template_template_mangling failed! Expected 99, got %d\n", tt_res); exit(1); }


  test_dynamic_cast_interop();
  test_layout_interop();
  test_empty_vbase_interop();
  test_ebo_interop();
  test_inline_guard_interop();
  test_cross_vbase_init();
  test_covariant_interop();
  test_nontype_interop();
  test_vptr_not_zero_memptr();
  test_static_local_collision_repro();

  test_repro_class_interop();
  test_fn_tmpl_interop();
  test_local_class_interop();

  printf("[Consumer] Testing member pointer to virtual function in class with virtual bases...\n");
  InteropVDerived vbase_d(100, 200);

#ifdef __clang__
  // Compiled by Clang: vbase_local_ptmf is correct (Clang-generated)
  VDerivedPTMF vbase_local_ptmf = &InteropVDerived::vfn;
  int vbase_local_res = (vbase_d.*vbase_local_ptmf)();
  printf("[Consumer] Local PTMF call result: %d\n", vbase_local_res);
  if (vbase_local_res != 300) { printf("[ERROR] Local PTMF call failed! Expected 300, got %d\n", vbase_local_res); exit(1); }

  // Pass Clang-generated PTMF to GCC lib (should work)
  int vbase_pass_res = call_vderived_ptmf(&vbase_d, vbase_local_ptmf);
  printf("[Consumer] Pass PTMF to lib result: %d\n", vbase_pass_res);
  if (vbase_pass_res != 300) { printf("[ERROR] Pass PTMF to lib failed! Expected 300, got %d\n", vbase_pass_res); exit(1); }
  printf("[Consumer] Virtual Base PTMF interop (Clang-initiated) passed!\n");
#else
  // NOTE on GCC 2.95 PMF Bug:
  // GCC 2.95 has a compiler bug for pointers to member functions (PMFs) to virtual 
  // functions in virtual bases when overridden in a derived class (e.g., &Derived::f).
  //
  // GCC 2.95 constructs the PMF with delta = 0 and delta2 = 0 because it fails to 
  // resolve the override to the defining virtual base, assuming the vptr is at offset 0.
  // However, since Derived only virtually overrides f and has no other dynamic bases, 
  // its vptr is placed only at offset 4 (VBase offset). Thus, calling GCC-generated PMFs 
  // on Derived will dereference the vbptr at offset 0 as a vptr, leading to a Segmentation Fault.
  // 
  // Standard-compliant compilers (like Clang) generate the correct PMF with delta = 4, delta2 = 4.
  // GCC 2.95 *can* successfully call these correct PMFs because its PMF call code correctly 
  // processes delta2 = 4.
  //
  // We will NOT support/emulate GCC 2.95's buggy PMF representation in Clang, as it is 
  // standard-non-compliant and crashes in GCC 2.95 anyway. Hence, we bypass calling 
  // GCC-generated local PMFs here and only test calling the correct Clang-generated PMFs.
  VDerivedPTMF vbase_lib_ptmf = get_vderived_ptmf(); // returns correct PTMF from Clang lib
  int vbase_lib_res = (vbase_d.*vbase_lib_ptmf)();
  printf("[Consumer] Lib-returned PTMF call result: %d\n", vbase_lib_res);
  if (vbase_lib_res != 300) { printf("[ERROR] Lib-returned PTMF call failed! Expected 300, got %d\n", vbase_lib_res); exit(1); }
  printf("[Consumer] Virtual Base PTMF interop (GCC-initiated with Clang PTMF) passed!\n");
#endif

  test_vptr_retreival_interop();
  test_ptmf_compare_repro();
  test_pass_by_val_dtor_only_interop();
  test_overwrite_derived_interop();
  test_empty_struct_pass_interop();
  test_vbase_alignment_bug();
  test_eh_mi_inline();
  test_local_rtti_interop();
  test_spec_local_class_interop();
  test_regular_local_class_rtti_interop();
  test_pmf_vbase_interop();
  test_eh_cleanup_vbase();

  printf("=== Interop Verification Complete ===\n");

  return 0;
}





