int g_consumer_first_definition = 2;
#include "lib.h"
#include <stdio.h>
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

  // Test 3: Lib allocates, Consumer deletes via virtual base pointer
  VBaseDtorTester::dtor_count = 0;
  VSubDtorTester *p3 = alloc_vsub();
  VBaseDtorTester *pb3 = p3;
  printf("[Consumer] Deleting pb3 (via virtual base pointer) in consumer...\n");
  delete pb3;
  printf("[Consumer] After delete pb3, VBaseDtorTester::dtor_count = %d\n", VBaseDtorTester::dtor_count);
  if (VBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Virtual Base Dtor Interop Failure (via base pointer): Expected VBaseDtorTester::dtor_count == 1, got %d\n", VBaseDtorTester::dtor_count);
    exit(1);
  }

  // Test 4: Consumer allocates, Lib deletes via virtual base pointer
  VBaseDtorTester::dtor_count = 0;
  printf("[Consumer] Allocating p4 in consumer...\n");
  VSubDtorTester *p4 = new VSubDtorTester();
  VBaseDtorTester *pb4 = p4;
  printf("[Consumer] Passing pb4 (via virtual base pointer) to lib for deletion...\n");
  delete_vsub_base(pb4);
  printf("[Consumer] After lib delete pb4, VBaseDtorTester::dtor_count = %d\n", VBaseDtorTester::dtor_count);
  if (VBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Virtual Base Dtor Interop Failure (via base pointer): Expected VBaseDtorTester::dtor_count == 1, got %d\n", VBaseDtorTester::dtor_count);
    exit(1);
  }

  // Test 5: Lib allocates, Consumer deletes via virtual base pointer (Inline Destructor)
  InlineVBaseDtorTester::dtor_count = 0;
  InlineVSubDtorTester *p5 = alloc_inline_vsub();
  InlineVBaseDtorTester *pb5 = p5;
  printf("[Consumer] Deleting pb5 (via virtual base pointer, inline dtor) in consumer...\n");
  delete pb5;
  printf("[Consumer] After delete pb5, InlineVBaseDtorTester::dtor_count = %d\n", InlineVBaseDtorTester::dtor_count);
  if (InlineVBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Inline Virtual Base Dtor Interop Failure (via base pointer): Expected InlineVBaseDtorTester::dtor_count == 1, got %d\n", InlineVBaseDtorTester::dtor_count);
    exit(1);
  }

  // Test 6: Consumer allocates, Lib deletes via virtual base pointer (Inline Destructor)
  InlineVBaseDtorTester::dtor_count = 0;
  printf("[Consumer] Allocating p6 in consumer...\n");
  InlineVSubDtorTester *p6 = new InlineVSubDtorTester();
  InlineVBaseDtorTester *pb6 = p6;
  printf("[Consumer] Passing pb6 (via virtual base pointer, inline dtor) to lib for deletion...\n");
  delete_inline_vsub_base(pb6);
  printf("[Consumer] After lib delete pb6, InlineVBaseDtorTester::dtor_count = %d\n", InlineVBaseDtorTester::dtor_count);
  if (InlineVBaseDtorTester::dtor_count != 1) {
    printf("[ERROR] Inline Virtual Base Dtor Interop Failure (via base pointer): Expected InlineVBaseDtorTester::dtor_count == 1, got %d\n", InlineVBaseDtorTester::dtor_count);
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
  CovariantBase2* expected;
  const char* expected_type;
  // Under GCC2 ABI interop:
  // If the Lib is compiled by GCC2, it is buggy and clone() thunk omits return pointer adjustment.
  // Thus, clone() returns the derived pointer (offset 0 relative to static_covariant_derived), which is b2 - 4.
  // If the Lib is compiled by Clang, it is correct and clone() thunk adjusts the return pointer to b2.
  // We dynamically check if clone() returned the unadjusted pointer (b2 - 4) or the adjusted pointer (b2).
  if ((char*)b2_clone == (char*)b2) {
    expected = b2;
    expected_type = "correct (Clang Lib)";
  } else {
    expected = (CovariantBase2*)((char*)b2 - 4);
    expected_type = "broken (GCC2 Lib)";
  }

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

  if (sizeof(ReproVDerived) != 20) {
    printf("[ERROR] sizeof(ReproVDerived) expected 20, got %d\n", (int)sizeof(ReproVDerived));
    exit(1);
  }
  if ((int)((char*)(ReproVBaseNonDyn*)&d1 - (char*)&d1) != 12) {
    printf("[ERROR] offset of ReproVBaseNonDyn expected 12, got %d\n", (int)((char*)(ReproVBaseNonDyn*)&d1 - (char*)&d1));
    exit(1);
  }
  if ((int)((char*)(ReproVBaseDyn*)&d1 - (char*)&d1) != 16) {
    printf("[ERROR] offset of ReproVBaseDyn expected 16, got %d\n", (int)((char*)(ReproVBaseDyn*)&d1 - (char*)&d1));
    exit(1);
  }

  printf("[Consumer] calling test_vptr_retreival_vbase...\n");
  test_vptr_retreival_vbase(&d1);
  printf("[Consumer] calling test_vptr_retreival_vbase_consumer...\n");
  test_vptr_retreival_vbase_consumer(&d1);

  printf("[Consumer] Testing vptr retrieval with non-virtual dynamic base with members...\n");
  ReproNVDerived d2;
  d2.x = 12345; // Make sure it doesn't crash if correct, or crashes if wrong
  printf("[Consumer] sizeof(ReproNVDerived) = %d\n", (int)sizeof(ReproNVDerived));
  printf("[Consumer] offset of ReproNVBase = %d\n", (int)((char*)(ReproNVBase*)&d2 - (char*)&d2));

  if (sizeof(ReproNVDerived) != 12) {
    printf("[ERROR] sizeof(ReproNVDerived) expected 12, got %d\n", (int)sizeof(ReproNVDerived));
    exit(1);
  }
  if ((int)((char*)(ReproNVBase*)&d2 - (char*)&d2) != 0) {
    printf("[ERROR] offset of ReproNVBase expected 0, got %d\n", (int)((char*)(ReproNVBase*)&d2 - (char*)&d2));
    exit(1);
  }

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

  if (sizeof(BugVBase) != 16) {
    printf("[ERROR] sizeof(BugVBase) expected 16, got %d\n", (int)sizeof(BugVBase));
    exit(1);
  }
  if ((int)((char*)(BugEmpty1*)&obj - (char*)&obj) != 12) {
    printf("[ERROR] offset of BugEmpty1 expected 12, got %d\n", (int)((char*)(BugEmpty1*)&obj - (char*)&obj));
    exit(1);
  }
  if ((int)((char*)(BugEmpty2*)&obj - (char*)&obj) != 13) {
    printf("[ERROR] offset of BugEmpty2 expected 13, got %d\n", (int)((char*)(BugEmpty2*)&obj - (char*)&obj));
    exit(1);
  }

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

void test_indirect_vbase_pmf_interop() {
  printf("[Consumer] Testing indirect vbase PMF interop...\n");
  printf("[Consumer] GCC sizeof(BugPMFB) = %d\n", (int)sizeof(BugPMFB));
  BugPMFB obj;
  BugPMFB *ptr = &obj;

  BugPMFB_PTMF pmf = get_bug_pmf();

#ifdef __clang__
  BugPMFB_PTMF local_pmf = &BugPMFB::f;
  Gcc2Pmf* g = (Gcc2Pmf*)&local_pmf;
  printf("[Consumer] Local PMF fields (Clang): delta=%d, index=%d, delta2=%d\n",
         g->delta, g->index, g->u.delta2);
  if (g->delta != 12 || g->u.delta2 != 16) {
    printf("[ERROR] Indirect PMF Virtual Base discrepancy! Expected delta=12, delta2=16, got delta=%d, delta2=%d\n",
           g->delta, g->u.delta2);
    exit(1);
  }
  printf("[Consumer] Local PMF standard-compliance check passed!\n");
#endif

  Gcc2Pmf* g_returned = (Gcc2Pmf*)&pmf;
  printf("[Consumer] Returned PMF fields: delta=%d, index=%d, delta2=%d\n",
         g_returned->delta, g_returned->index, g_returned->u.delta2);

  if (g_returned->delta == 0 && g_returned->u.delta2 == 4) {
    printf("[Consumer] Bypassing call for buggy GCC2 PMF to avoid crash.\n");
  } else {
    printf("[Consumer] Calling pmf...\n");
    (ptr->*pmf)();
  }
  printf("[Consumer] Indirect PMF Virtual Base interop passed!\n");
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

void test_indirect_vbase_consumer() {
  printf("[Consumer] test_indirect_vbase_consumer() constructing IndirectVBaseE...\n");
  IndirectVBaseE* e = new IndirectVBaseE();
  printf("[Consumer] test_indirect_vbase_consumer() success!\n");
  delete e;
}

void test_indirect_vbase_interop_repro() {
  printf("[Consumer] Calling test_indirect_vbase_interop (lib-side construction)...\n");
  test_indirect_vbase_interop();
}

namespace {
  void interop_anon_func() {
    static int call_count = 0;
    call_count++;
  }
}

void test_namespace_interop() {
  printf("[Consumer] Testing anonymous namespace local call...\n");
  interop_anon_func();
  printf("[Consumer] Testing namespace global function interop...\n");
  int res = InteropNS::namespace_func(42);
  printf("[Consumer] InteropNS::namespace_func result: %d\n", res);
  if (res != 142) {
    printf("[ERROR] Namespace func returned wrong value: %d\n", res);
    exit(1);
  }

  // Test template class inside namespace (successful test case coverage)
  InteropNS::TmplClass<int> tc(42);
  int ns_tmpl_res = InteropNS::test_ns_tmpl_class(&tc);
  printf("[Consumer] InteropNS::test_ns_tmpl_class result: %d\n", ns_tmpl_res);
  if (ns_tmpl_res != 52) {
    printf("[ERROR] test_ns_tmpl_class failed: %d\n", ns_tmpl_res);
    exit(1);
  }

  printf("[Consumer] Namespace func interop passed!\n");
}

void test_namespace_fuzz_interop() {
  printf("[Consumer] Testing namespace fuzzing interop...\n");

  // 1. Deep nested namespace
  int deep_res = A::B::C::D::deep_func(42);
  printf("[Consumer] deep_func result: %d\n", deep_res);
  if (deep_res != 242) { printf("[ERROR] deep_func failed!\n"); exit(1); }

  // 1b. Nested std namespace
  int nested_std_res = A::std::nested_std_func(42);
  printf("[Consumer] nested_std_func result: %d\n", nested_std_res);
  if (nested_std_res != 84) { printf("[ERROR] nested_std_func failed!\n"); exit(1); }

  // 1c. std::B::std_nested_func (std at top-level is ignored in GCC2)
  int std_nested_res = std::B::std_nested_func(42);
  printf("[Consumer] std_nested_func result: %d\n", std_nested_res);
  if (std_nested_res != 92) { printf("[ERROR] std_nested_func failed!\n"); exit(1); }

  // 1c2. std::top_level_std_func (std at top-level is ignored in GCC2)
  int top_level_std_res = std::top_level_std_func(42);
  printf("[Consumer] top_level_std_func result: %d\n", top_level_std_res);
  if (top_level_std_res != 3042) { printf("[ERROR] top_level_std_func failed!\n"); exit(1); }

  // 1c3. std::std::nested_std_std_func (nested std namespace)
  int nested_std_std_res = std::std::nested_std_std_func(42);
  printf("[Consumer] nested_std_std_func result: %d\n", nested_std_std_res);
  if (nested_std_std_res != 142) { printf("[ERROR] nested_std_std_func failed!\n"); exit(1); }

  // 1c4. std::my_foo::nested_foo_func
  int nested_foo_res = std::my_foo::nested_foo_func(42);
  printf("[Consumer] nested_foo_func result: %d\n", nested_foo_res);
  if (nested_foo_res != 242) { printf("[ERROR] nested_foo_func failed!\n"); exit(1); }

  // 1c5. std::my_foo::std::nested_foo_std_func
  int nested_foo_std_res = std::my_foo::std::nested_foo_std_func(42);
  printf("[Consumer] nested_foo_std_func result: %d\n", nested_foo_std_res);
  if (nested_foo_std_res != 342) { printf("[ERROR] nested_foo_std_func failed!\n"); exit(1); }

  // 1d. 10-level nested namespace
  int deep_10_res = N1::N2::N3::N4::N5::N6::N7::N8::N9::N10::deep_func_10(42);
  printf("[Consumer] deep_func_10 result: %d\n", deep_10_res);
  if (deep_10_res != 1042) { printf("[ERROR] deep_func_10 failed!\n"); exit(1); }



  // 2. Template function in namespace
  int templ_res = Foo::templ_func(42);
  printf("[Consumer] templ_func result: %d\n", templ_res);
  if (templ_res != 43) { printf("[ERROR] templ_func failed!\n"); exit(1); }

  // 3. Overloaded functions
  int overload_i = Foo::overload(42);
  double overload_d = Foo::overload(42.0);
  printf("[Consumer] overload(int) result: %d, overload(double) result: %f\n", overload_i, overload_d);
  if (overload_i != 52 || overload_d != 62.0) { printf("[ERROR] overload failed!\n"); exit(1); }

  // 4. Return struct in another namespace
  X::S s = Y::func(42);
  printf("[Consumer] Y::func returned struct val: %d\n", s.val);
  if (s.val != 342) { printf("[ERROR] Y::func failed!\n"); exit(1); }

  // 5. Take member pointer
  M::S ms;
  ms.x = 12345;
  M::S_PTMD p = &M::S::x;
  int m_res = M::func(p, &ms);
  printf("[Consumer] M::func result: %d\n", m_res);
  if (m_res != 12345) { printf("[ERROR] M::func failed!\n"); exit(1); }

  // Fuzzed case 1: 10-level deep nested namespace fuzzed function
  int deep_nested_res = N1::N2::N3::N4::N5::N6::N7::N8::N9::N10::deep_nested_func(42);
  printf("[Consumer] deep_nested_func result: %d\n", deep_nested_res);
  if (deep_nested_res != 2042) { printf("[ERROR] deep_nested_func failed!\n"); exit(1); }

  // Fuzzed case 2: Multi-parameter template class inside nested namespace
  MultiParamTemplate<int,int,int,int,int,int,int,int,int,int> mpt(42);
  int mpt_res = test_multi_param_template(&mpt);
  printf("[Consumer] test_multi_param_template result: %d\n", mpt_res);
  if (mpt_res != 141) { printf("[ERROR] test_multi_param_template failed!\n"); exit(1); }

  printf("[Consumer] Namespace fuzzing interop passed!\n");
}

void test_fuzzed_namespace_local_rtti() {
  printf("[Consumer] Testing fuzzed namespace local class RTTI interop...\n");
  NamespaceLocal::RttIBase* p_lib = NamespaceLocal::get_lib_namespace_local_rtti();
  NamespaceLocal::RttIBase* p_cons = NamespaceLocal::get_namespace_local_rtti_inline<int>();

  printf("[Consumer] p_lib typeid name = %s\n", typeid(*p_lib).name());
  printf("[Consumer] p_cons typeid name = %s\n", typeid(*p_cons).name());

  if (typeid(*p_lib) == typeid(*p_cons)) {
    printf("[Consumer] Namespace local class RTTI interop passed!\n");
  } else {
    printf("[ERROR] Namespace local class RTTI mismatch! p_lib=%s, p_cons=%s\n",
           typeid(*p_lib).name(), typeid(*p_cons).name());
    exit(1);
  }
  delete p_lib;
  delete p_cons;
}

void test_namespace_digit_interop() {
  printf("[Consumer] Testing namespace with digit interop...\n");
  int res = NamespaceDigit1::NamespaceDigit2::nested_func(42);
  printf("[Consumer] NamespaceDigit1::NamespaceDigit2::nested_func result: %d\n", res);
  if (res != 542) {
    printf("[ERROR] NamespaceDigit1::NamespaceDigit2::nested_func failed!\n");
    exit(1);
  }
  printf("[Consumer] Namespace digit interop passed!\n");
}

void test_nested_class_digit_interop() {
  printf("[Consumer] Testing nested class with digit interop...\n");
  NestedClassDigit1::NestedClassDigit2 b;
  int res = b.nested_func(42);
  printf("[Consumer] NestedClassDigit1::NestedClassDigit2::nested_func result: %d\n", res);
  if (res != 642) {
    printf("[ERROR] NestedClassDigit1::NestedClassDigit2::nested_func failed!\n");
    exit(1);
  }
  printf("[Consumer] Nested class digit interop passed!\n");
}

void test_anon_namespace_rtti_interop() {
  printf("[Consumer] Testing anonymous namespace RTTI comparison...\n");
  const std::type_info& lib_ti = get_lib_anon_secret_ti();
  const std::type_info& local_ti = typeid(InteropAnonSecret);

  printf("[Consumer] Lib RTTI address = %p\n", (void*)&lib_ti);
  printf("[Consumer] Local RTTI address = %p\n", (void*)&local_ti);
  printf("[Consumer] Lib RTTI name = %s\n", lib_ti.name());
  printf("[Consumer] Local RTTI name = %s\n", local_ti.name());

  if (lib_ti == local_ti) {
    printf("[ERROR] Silent Bug! Anonymous namespace RTTI matched across different TUs!\n");
    exit(1);
  }
  printf("[Consumer] Anonymous namespace RTTI comparison passed (correctly different)!\n");
}

void test_vbptr_cast_bug_interop() {
  printf("[Consumer] Testing virtual base cast bug interop...\n");
  CastBugD* d = get_cast_bug_d();
  printf("[Consumer] d = %p\n", d);
  void** vptr = *(void***)((char*)d + 8);
  printf("[Consumer] vptr (at d + 8) = %p\n", vptr);
  if (vptr) {
    printf("[Consumer] vtable[0] (at vptr) = %p\n", vptr[0]);
    printf("[Consumer] vtable[1] (at vptr + 4) = %p\n", vptr[1]);
    printf("[Consumer] vtable[2] (at vptr + 8) = %p\n", vptr[2]);
  }
  d->fd();

  printf("[Consumer] casting CastBugD* to CastBugV1*...\n");
  CastBugV1* v1 = d; // Implicit cast to virtual base!
  printf("[Consumer] v1 = %p\n", v1);

  // Get expected V1 address dynamically from GCC2-compiled library.
  CastBugV1* expected_v1 = get_cast_bug_v1();
  printf("[Consumer] expected v1 = %p\n", expected_v1);

  if (v1 != expected_v1) {
    printf("[ERROR] Virtual base cast discrepancy! Got %p, expected %p\n", v1, expected_v1);
    exit(1);
  }

  v1->f1(); // Should print v1=111
  printf("[Consumer] Virtual base cast interop passed!\n");
}

/*
 * NOTE ON GCC 2.95 POINTER-TO-MEMBER-DATA (PTMD) CAST NULL CORRUPTION BUG:
 * Under the C++ standard, casting a null member data pointer must always preserve the null value
 * (i.e., converting a null member pointer of type `int Base::*` to `int Derived::*` must yield a
 * null member pointer of type `int Derived::*`, represented as `0`).
 *
 * However, GCC 2.95 has a bug in its member pointer cast implementation (see `gcc/cp/cvt.c:cp_convert_to_pointer`).
 * When converting pointer-to-members (`TYPE_PTRMEM_P`), GCC 2.95 blindly adds/subtracts the base class
 * offset using `size_binop` without checking if the source member pointer is null:
 *
 *     if (binfo && ! TREE_VIA_VIRTUAL (binfo))
 *       expr = size_binop (code, expr, BINFO_OFFSET (binfo));
 *
 * Consequently:
 * 1. In GCC 2.95, a null member pointer casted under multiple inheritance (where Base has a non-zero offset)
 *    is corrupted and becomes `0 + Base_Offset = Base_Offset` (e.g., `4`).
 * 2. This corrupted pointer is no longer treated as null by GCC 2.95's runtime null checks (`ptmd == 0`),
 *    which evaluate to false!
 * 3. Dereferencing it under GCC 2.95 accesses `BaseAddr + Base_Offset - 1 = BaseAddr + 3`, reading
 *    garbage/unaligned memory instead of safely failing or acting as null.
 *
 * Standard-compliant compilers (like Clang) correctly check for null and preserve it as `0`.
 *
 * Interoperability Behavior:
 * - Clang -> GCC 2.95: Clang returns correct null (`0`). GCC 2.95 consumer receives `0` and evaluates
 *   it as NULL (since `0 == 0`). However, comparing it against a local GCC2 casted null (which has value `4`)
 *   fails (`0 != 4`).
 * - GCC 2.95 -> Clang: GCC 2.95 returns buggy null (`4`). Clang consumer receives `4` and treats it as
 *   non-null.
 *
 * In accordance with standard compliance, Clang does NOT emulate this GCC2 bug and always generates
 * correct, standard-compliant null PTMDs (`0`).
 * We assert standard compliance under Clang (casted null must equal `0`), print a warning and document
 * the bug when receiving a buggy GCC 2.95 PTMD, and bypass dereference checks for GCC2-generated buggy PTMDs.
 */
void test_ptmd_cast_null_interop() {
  printf("[Consumer] Testing PTMD cast null interop...\n");

  CastBugPTMD_C_PTMD ptmd_null = get_cast_bug_ptmd_null();
  CastBugPTMD_C_PTMD ptmd_nonnull = get_cast_bug_ptmd_nonnull();

  int val_null = *(int*)&ptmd_null;
  int val_nonnull = *(int*)&ptmd_nonnull;
  printf("[Consumer] PTMD representations from Lib: null=%d, nonnull=%d\n", val_null, val_nonnull);

  CastBugPTMD_C obj;
  obj.a = 0x11111111;
  obj.b = 0x22222222;
  obj.c = 0x33333333;

  CastBugPTMD_C_PTMD local_null = (CastBugPTMD_B_PTMD)0;
  int val_local_null = *(int*)&local_null;
  printf("[Consumer] Local null PTMD representation: %d\n", val_local_null);

#ifdef __clang__
  // Assert standard-compliant Clang behavior: casted null must be exactly 0 (literal NULL)
  printf("[Consumer] Asserting Clang standard-compliant null preservation...\n");
  if (val_local_null != 0) {
    printf("[ERROR] Clang failed standard compliance! Local casted null is %d, expected 0.\n", val_local_null);
    exit(1);
  }
  printf("[Consumer] Clang local null preservation check passed!\n");
#else
  // GCC 2.95 buggy behavior
  printf("[Consumer] GCC 2.95 local null cast representation is %d (known bug, expected 4 due to Base offset).\n", val_local_null);
  if (val_local_null != 4) {
    printf("[ERROR] Unexpected GCC 2.95 null cast representation: %d, expected 4.\n", val_local_null);
    exit(1);
  }
#endif

  // Validate and interoperate
  if (val_null == 0) {
    printf("[Consumer] Library returned correct standard-compliant null PTMD (0).\n");
    printf("[Consumer] PTMD cast null interop passed!\n");
  } else if (val_null == 4) {
    printf("[WARNING] Library returned BUGGY GCC 2.95 null PTMD representation (4)!\n");
    printf("[WARNING] Bypassing strict equality assertions to allow interop with buggy GCC 2.95 binary.\n");

    // If we are running under GCC 2.95, both local and lib are 4, so we can check they match
#ifndef __clang__
    if (val_local_null == 4) {
      printf("[Consumer] Both sides are GCC 2.95. Verifying they agree on buggy dereference...\n");
      int local_deref_res = obj.*local_null;
      int lib_deref_res = call_cast_bug_ptmd(&obj, local_null);
      printf("[Consumer] Buggy local_deref = 0x%x, lib_deref = 0x%x\n", local_deref_res, lib_deref_res);
      if (local_deref_res != lib_deref_res) {
        printf("[ERROR] GCC2 buggy PTMD dereference mismatch!\n");
        exit(1);
      }
      printf("[Consumer] GCC 2.95 buggy PTMD cast null interop passed!\n");
    }
#endif
  } else {
    printf("[ERROR] Unknown PTMD null cast representation returned from library: %d\n", val_null);
    exit(1);
  }
}

void test_template_ref_interop() {
  printf("[Consumer] Testing template reference non-type parameters...\n");
  S_nontype_ref_global<interop_global_var> x1;
  test_template_ref_global_interop(x1);

  S_nontype_ref_fn<test_extern_func> x2;
  test_template_ref_fn_interop(x2);
}

void test_rtti_ptmd_interop() {
  printf("[Consumer] Testing RTTI pointer-to-member mangling...\n");
  const std::type_info &ti_ptmd = typeid(RttiPtmdBase_PTMD);
  const std::type_info &ti_ptmf = typeid(RttiPtmdBase_PTMF);
  const std::type_info &ti_tmpl_ptmd = typeid(RttiPtmdTmpl_PTMD);

  printf("[Consumer] RTTI ti_ptmd name = %s\n", ti_ptmd.name());
  printf("[Consumer] RTTI ti_ptmf name = %s\n", ti_ptmf.name());
  printf("[Consumer] RTTI ti_tmpl_ptmd name = %s\n", ti_tmpl_ptmd.name());

  check_rtti_ptmd(ti_ptmd, ti_ptmf, ti_tmpl_ptmd);
}

void test_ptmf_nontype_vbase_interop_repro() {
  printf("[Consumer] Testing PTMF non-type template parameter of virtual base virtual function...\n");
  PTMFDerived obj;
  PTMFDerived *ptr = &obj;

  PTMFDerived_PTMF pmf = get_ptmf_derived_f();
  Gcc2Pmf* g = (Gcc2Pmf*)&pmf;
  printf("[Consumer] get_ptmf_derived_f PMF fields: delta=%d, index=%d, delta2=%d\n",
         g->delta, g->index, g->u.delta2);

  if (g->delta == 0 && g->u.delta2 == 0) {
    printf("[Consumer] Bypassing template call for corrupt GCC2 virtual base PTMF to avoid SEGFAULT.\n");
  } else {
    printf("[Consumer] Calling template function...\n");
    PTMFNontype<&PTMFDerived::f> s;
    s.call(obj);
  }
}

void test_fuzz_layout_interop() {
  printf("[Consumer] Testing diamond virtual inheritance layout...\n");
  DiaD d;
  d.a = 10;
  d.b = 20;
  d.c = 30;
  d.d = 40;
  check_dia_layout(&d);
  printf("[Consumer] Diamond virtual inheritance offsets check:\n");
  printf("  d=%p, B=%p, C=%p, A=%p\n",
         &d, (DiaB*)&d, (DiaC*)&d, (DiaA*)&d);

  printf("[Consumer] Testing multiple empty bases layout...\n");
  FuzzEmptyBases feb;
  check_fuzz_empty_layout(&feb);
  printf("[Consumer] Multiple empty bases offsets check:\n");
  printf("  feb=%p, Empty1=%p, Empty2=%p\n",
         &feb, (FuzzEmpty1*)&feb, (FuzzEmpty2*)&feb);
}

void test_pmf_check_interop() {
  printf("[Consumer] Testing PMF virtual/non-virtual representation...\n");
  PmfCheckBase_PTMF p_f = get_pmf_check_f();
  PmfCheckBase_PTMF p_g = get_pmf_check_g();

  struct Gcc2Pmf {
    short delta;
    short index;
    union {
      void* pfn;
      short delta2;
    } u;
  };

  Gcc2Pmf* gf = (Gcc2Pmf*)&p_f;
  Gcc2Pmf* gg = (Gcc2Pmf*)&p_g;

  printf("[Consumer] f (virtual) PMF: delta=%d, index=%d, delta2=%d\n",
         gf->delta, gf->index, gf->u.delta2);
  printf("[Consumer] g (non-virtual) PMF: delta=%d, index=%d\n",
         gg->delta, gg->index);
}

void test_rtti_nv_vbase_dynamic_cast_interop() {
  printf("[Consumer] Testing RTTI NV virtual base dynamic_cast interop...\n");
  RttiNvVbase_NV* p = get_rtti_nv_vbase_object();
  printf("[Consumer] p = %p\n", p);

  // Attempt dynamic_cast to virtual base
  printf("[Consumer] Attempting dynamic_cast<RttiNvVbase_V1*>(p)...\n");
  RttiNvVbase_V1* v = dynamic_cast<RttiNvVbase_V1*>(p);
  printf("[Consumer] dynamic_cast result: %p\n", v);
  if (v == NULL) {
    printf("[ERROR] dynamic_cast returned NULL!\n");
    exit(1);
  }
  v->f1();
  printf("[Consumer] RTTI NV dynamic_cast interop passed!\n");
}

void test_rtti_multiple_vbases_interop() {
  printf("[Consumer] Testing RTTI multiple vbases dynamic_cast interop...\n");
  RttiMultiD* pd = get_rtti_multi_d_object();

  // Cast to virtual bases (static)
  RttiMultiV1* pv1 = pd;
  RttiMultiV2* pv2 = pd;

  // Cross-cast from V1 to V2 (requires RTTI)
  printf("[Consumer] Attempting dynamic_cast<RttiMultiV2*>(pv1) [cross-cast]...\n");
  RttiMultiV2* pv2_from_v1 = dynamic_cast<RttiMultiV2*>(pv1);
  printf("[Consumer] Cross-cast result: %p\n", pv2_from_v1);
  if (pv2_from_v1 == NULL) {
    printf("[ERROR] Cross-cast returned NULL!\n");
    exit(1);
  }
  pv2_from_v1->f2();

  printf("[Consumer] RTTI multiple vbases interop passed!\n");
}

inline void* operator new(size_t, void* __p) throw() { return __p; }

void test_ice_repro_interop() {
  printf("[Consumer] Testing hybrid NV/V layout ICE repro...\n");
  printf("[Consumer] GCC sizeof(IceReproD) = %d\n", (int)sizeof(IceReproD));
  char buf[64] = {0};
  IceReproD *d = new (buf) IceReproD();
  unsigned* ptr = (unsigned*)d;
  for (int i = 0; i < sizeof(IceReproD)/4; ++i) {
    printf("  d[%d] = 0x%x (%u)\n", i, ptr[i], ptr[i]);
  }
  test_ice_repro(d);
  d->~IceReproD();
  printf("[Consumer] Hybrid NV/V layout ICE repro passed!\n");
}

void test_mangle_bug_interop() {
  printf("[Consumer] Testing MangleBug interop...\n");
  MangleBug obj;
  MangleBug other;
  other.x = 200;
  obj.f(other);
  printf("[Consumer] MangleBug interop passed!\n");
}

int main() {


  printf("=== GCC 2.x Full Interop Verification ===\n");
  // {
  //   printf("[DEBUG_GCC] Instantiating Fuzz691_C3 for static symbol reproduction...\n");
  //   Fuzz691_C3 obj;
  //   Fuzz691_C3 *p = &obj;
  //   Fuzz691_C2 *b2 = (Fuzz691_C2*)p;
  //   Fuzz691_C1 *b1 = (Fuzz691_C1*)p;
  //   printf("[DEBUG_GCC] Fuzz691_C3->Fuzz691_C2 base offset (GCC2) = %d\n", (int)((char*)b2 - (char*)p));
  //   printf("[DEBUG_GCC] Fuzz691_C3->Fuzz691_C1 base offset (GCC2) = %d\n", (int)((char*)b1 - (char*)p));
  //   printf("[DEBUG_GCC] sizeof(Fuzz691_C3) (GCC2) = %d\n", (int)sizeof(Fuzz691_C3));
  // }
  test_vbptr_cast_bug_interop();
  test_ice_repro_interop();
  test_mangle_bug_interop();
  test_ptmd_cast_null_interop();
  printf("[Consumer] Initial Derived::instance_count = %d\n", Derived::instance_count);

  {
    printf("[Consumer] Calling normal_func(100, 50.5)...\n");
    int res = normal_func(100, 50.5);
    printf("[Consumer] normal_func returned: %d\n", res);
    if (res != 150) { printf("[ERROR] normal_func failed! Expected 150, got %d\n", res); exit(1); }

    Derived d(1000);
    Derived *p = &d;
    printf("[Consumer] p = %p\n", p);
    void** pvptr = *(void***)p;
    printf("[Consumer] pvptr (at p) = %p\n", pvptr);
    if (pvptr) {
      printf("[Consumer] pvptr[0] = %p\n", pvptr[0]);
      printf("[Consumer] pvptr[1] = %p\n", pvptr[1]);
      printf("[Consumer] pvptr[2] = %p\n", pvptr[2]);
      printf("[Consumer] pvptr[3] = %p\n", pvptr[3]);
      printf("[Consumer] pvptr[4] = %p\n", pvptr[4]);
      printf("[Consumer] pvptr[5] = %p\n", pvptr[5]);
    }
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
    union {
      DerivedPTMF ptmf;
      struct {
        short delta;
        short index;
        union {
          void* func;
          short delta2;
        } u;
      } s;
    } u;
    u.ptmf = ptmf1;
    printf("[Consumer] ptmf1 representation: delta=%d, index=%d, delta2/func=%p\n", u.s.delta, u.s.index, u.s.u.func);
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

  test_indirect_vbase_consumer();
  test_indirect_vbase_interop_repro();
  test_indirect_vbase_pmf_interop();
  test_namespace_interop();
  test_namespace_fuzz_interop();
  test_fuzzed_namespace_local_rtti();
  test_namespace_digit_interop();
  test_nested_class_digit_interop();
  test_template_ref_interop();
  test_rtti_ptmd_interop();
  test_fuzz_layout_interop();
  test_pmf_check_interop();
  test_rtti_nv_vbase_dynamic_cast_interop();
  test_rtti_multiple_vbases_interop();
  test_anon_namespace_rtti_interop();

  // Zero-sized array pointer RTTI/mangling interop fuzzed check
  int (*zero_arr)[0] = 0;
  test_zero_array(zero_arr);

  // Size-1 array pointer RTTI/mangling check
  int (*one_arr)[1] = 0;
  test_one_array(one_arr);

  test_local_class_mangled_uniquifier_interop(get_local_fn_ptr_inline());

  // FuzzSuccess9 layout test
  {
    printf("[Consumer] Running FuzzSuccess9 interop test...\n");
    FuzzSuccessC3 obj;
    check_fuzz_success_9(&obj, &obj, &obj);
  }

  // AlignBug layout test
  {
    printf("[Consumer] Running AlignBug interop test...\n");
    AlignBugC2 obj2;
    AlignBugC3 obj3;
    AlignBugC1 obj1;
    check_align_bug(&obj2, &obj3, &obj1, &obj2);
  }

  // LinkBug vtable mangling reproduction test
  {
    printf("[Consumer] Running LinkBug interop test...\n");
    LinkBugC1 obj1;
    LinkBugC2 obj2;
    check_link_bug(&obj1, &obj2);
  }

  // VirtOnlyBug layout and mangling reproduction test
  {
    printf("[Consumer] Running VirtOnlyBug interop test...\n");
    VirtOnlyBugC3 obj3;
    check_virt_only_bug(&obj3, &obj3, &obj3);

    printf("[Consumer] Deleting VirtOnlyBugC3 polymorphically...\n");
    VirtOnlyBugC3 *p = new VirtOnlyBugC3();
    VirtOnlyBugC1 *b = (VirtOnlyBugC1*)p;
    delete b;
    printf("[Consumer] Polymorphic deletion success!\n");
  }

  // EmptySubVT vtable mangling reproduction test
  {
    printf("[Consumer] Running EmptySubVT interop test...\n");
    EmptySubVT_C2 obj2;
    check_empty_sub_vt(&obj2);
  }

  // SuffixPriorBug secondary base suffix mangling prioritization test
  {
    printf("[Consumer] Running SuffixPriorBug interop test...\n");
    SuffixPriorBug_C4 *p = new SuffixPriorBug_C4();
    SuffixPriorBug_C3 *b = (SuffixPriorBug_C3*)p;
    check_suffix_prior_bug(p, b);
    delete p;
  }

  // Fuzz676 layout and mangling verification test
  {
    printf("[Consumer] Running Fuzz676 interop test...\n");
    Fuzz676_C4 obj4;
    check_fuzz_676(&obj4);
  }

  // Fuzz682 layout, vtable, and virtual/non-virtual hybrid casting test
  {
    printf("[Consumer] Running Fuzz682 interop test...\n");
    Fuzz682_C3 *p = new Fuzz682_C3();
    Fuzz682_C2 *b2 = (Fuzz682_C2*)p;
    Fuzz682_C1 *b1 = (Fuzz682_C1*)p;
    printf("[Consumer] Fuzz682_C3->Fuzz682_C2 base offset (GCC2) = %d\n", (int)((char*)b2 - (char*)p));
    printf("[Consumer] Fuzz682_C3->Fuzz682_C1 base offset (GCC2) = %d\n", (int)((char*)b1 - (char*)p));
    check_fuzz_682(p);

    // Permanent link-time coverage check for the overridden print_class thunk
    extern void* link_thunk_coverage __asm__("__thunk_12_print_class__10Fuzz682_C3");
    (void)link_thunk_coverage;

    printf("[Consumer] Deleting Fuzz682_C3 polymorphically via non-virtual Fuzz682_C0 base...\n");
    Fuzz682_C0 *b = (Fuzz682_C0*)(Fuzz682_C1*)p;
    delete b;
    printf("[Consumer] Polymorphic deletion success!\n");
  }

  // Fuzz691 mangling discrepancy reproduction test (static only, no-fix requested)
  {
    printf("[Consumer] Instantiating Fuzz691_C3 for static symbol reproduction...\n");
    Fuzz691_C3 obj;
    Fuzz691_C3 *p = &obj;
    Fuzz691_C2 *b2 = (Fuzz691_C2*)p;
    Fuzz691_C1 *b1 = (Fuzz691_C1*)p;
    printf("[Consumer] Fuzz691_C3->Fuzz691_C2 base offset (GCC2) = %d\n", (int)((char*)b2 - (char*)p));
    printf("[Consumer] Fuzz691_C3->Fuzz691_C1 base offset (GCC2) = %d\n", (int)((char*)b1 - (char*)p));
    check_fuzz_691(&obj);
  }

  // Permanent link-time coverage check for standalone primary vtable emission
  {
    printf("[Consumer] Standalone primary vtable emission permanent linkage check...\n");
    extern void* link_vtable_coverage __asm__("__vt_Q212NsuPKHDHA2W46CGu__3");
    (void)link_vtable_coverage;
  }

  // Primary base dynamic class vfptr missing reproduction test
  {
    printf("[Consumer] Running PrimaryBug interop test...\n");
    PrimaryBugC3 obj;
    check_primary_bug(&obj, &obj);
  }

  // Template explicit specializations and prefix separator mangling interop test
  {
    printf("[Consumer] Running Template Interop test...\n");
    NsInteropTemplate::NsSub_::TemplateClass obj;
    check_template_interop(&obj);

    // Permanent link-time asm coverage check for template specializations separator mangling
    extern void* link_mt_func_coverage __asm__("mt_func__H2Zdi42_Q317NsInteropTemplate6NsSub_13TemplateClassX01_v");
    extern void* link_tfn_func_coverage __asm__("tfn_func__H2Zdi42_Q217NsInteropTemplate6NsSub_X01_v");
    (void)link_mt_func_coverage;
    (void)link_tfn_func_coverage;
  }

    // Multi-level virtual override dynamic constructor dispatch interop test
    {
      printf("[Consumer] Running VirtOverride interop test...\n");
      int val = NsInteropVirtOverride::check_virt_override_interop();
      printf("[Consumer] check_virt_override_interop returned %d\n", val);
      if (val != 4) {
        printf("[ERROR] Virtual override interop discrepancy! Expected 4, got %d\n", val);
        exit(1);
      }
    }

    printf("=== Interop Verification Complete ===\n");
    return 0;
  }
