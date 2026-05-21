#!/bin/bash
set -e

SCRATCH_DIR="."
GCC_DIR=".gcc-2.95"
OPT_LEVEL="${1:--O0}"

CLANGPP="build/bin/clang++ --target=i386-pc-linux-gnu -fc++-abi=gcc2 -nostartfiles -nodefaultlibs -Wl,--no-eh-frame-hdr -fexceptions -fcxx-exceptions -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/Scrt1.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crti.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtbeginS.o"
GPP="$GCC_DIR/usr/bin/g++-2.95 -Wa,--32 -B$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/ $OPT_LEVEL -fpermissive -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"
CLANG_CC1="build/bin/clang -cc1 -triple i386-pc-linux-gnu -fc++-abi=gcc2 -std=c++98 -fexceptions -fcxx-exceptions $OPT_LEVEL -D__extern_always_inline=__inline__ -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"
CLANG_CC1_CXX20="build/bin/clang -cc1 -triple i386-pc-linux-gnu -fc++-abi=gcc2 -std=c++20 -fno-sized-deallocation -Wno-dynamic-exception-spec -fexceptions -fcxx-exceptions $OPT_LEVEL -D__extern_always_inline=__inline__ -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"

echo "============================================================"
echo "   LEGACY GCC 2.95 & CLANG GCC2 ABI INTEROP VERIFICATION    "
echo "   Optimization Level: $OPT_LEVEL                           "
echo "============================================================"
echo "Cleaning up stale build artifacts..."
rm -f $SCRATCH_DIR/app_comb* $SCRATCH_DIR/app_cpp20_comb* $SCRATCH_DIR/*.o $SCRATCH_DIR/*.ll


echo ""
echo "------------------------------------------------------------"
echo " COMBINATION 1: Build LIB with Clang, CONSUMER with GCC 2.95"
echo "------------------------------------------------------------"

$CLANG_CC1 -emit-obj $SCRATCH_DIR/lib.cpp -o $SCRATCH_DIR/lib_clang.o

$GPP -c $SCRATCH_DIR/consumer.cpp -o $SCRATCH_DIR/consumer_gcc.o

$CLANGPP $SCRATCH_DIR/lib_clang.o $SCRATCH_DIR/consumer_gcc.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_comb1

echo "[SUCCESS] Combination 1 compiled and linked successfully."
echo "Running Combination 1:"
$SCRATCH_DIR/app_comb1

echo ""
echo "------------------------------------------------------------"
echo " COMBINATION 2: Build LIB with GCC 2.95, CONSUMER with Clang"
echo "------------------------------------------------------------"

$GPP -c $SCRATCH_DIR/lib.cpp -o $SCRATCH_DIR/lib_gcc.o

$CLANG_CC1 -emit-obj $SCRATCH_DIR/consumer.cpp -o $SCRATCH_DIR/consumer_clang.o

echo "Verifying local class mangling inside explicit template specialization..."
if ! nm $SCRATCH_DIR/consumer_clang.o | grep -q "get_val__Q235test_spec_local_class__H1Zi_X01_i.0_5Local"; then
  echo "[ERROR] Mangling discrepancy in Clang's GCC2 ABI!"
  echo "Expected local class member symbol: get_val__Q235test_spec_local_class__H1Zi_X01_i.0_5Local"
  echo "Actual symbols in consumer_clang.o:"
  nm $SCRATCH_DIR/consumer_clang.o | grep "Local" || true
  exit 1
fi
echo "[SUCCESS] Local class mangling matched GCC 2.95!"


$CLANGPP $SCRATCH_DIR/lib_gcc.o $SCRATCH_DIR/consumer_clang.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_comb2

echo "[SUCCESS] Combination 2 compiled and linked successfully."
echo "Running Combination 2:"
$SCRATCH_DIR/app_comb2

echo ""
echo "------------------------------------------------------------"
echo " ISOLATED C++20 CODENAME: FLOAT TEMPLATE PARAMS INTEROP      "
echo "------------------------------------------------------------"

# Option A: Lib in Clang (C++20), Consumer in GCC 2.95 (C++98)
$CLANG_CC1_CXX20 -emit-obj $SCRATCH_DIR/lib_cpp20.cpp -o $SCRATCH_DIR/lib_cpp20_clang.o
$GPP -c $SCRATCH_DIR/consumer_cpp20.cpp -o $SCRATCH_DIR/consumer_cpp20_gcc.o

$CLANGPP $SCRATCH_DIR/lib_cpp20_clang.o $SCRATCH_DIR/consumer_cpp20_gcc.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_cpp20_comb1

echo "[SUCCESS] Isolated C++20 Combination 1 compiled and linked."
$SCRATCH_DIR/app_cpp20_comb1

# Option B: Lib in GCC 2.95 (C++98), Consumer in Clang (C++20)
$GPP -c $SCRATCH_DIR/lib_cpp20.cpp -o $SCRATCH_DIR/lib_cpp20_gcc.o
$CLANG_CC1_CXX20 -emit-obj $SCRATCH_DIR/consumer_cpp20.cpp -o $SCRATCH_DIR/consumer_cpp20_clang.o

$CLANGPP $SCRATCH_DIR/lib_cpp20_gcc.o $SCRATCH_DIR/consumer_cpp20_clang.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_cpp20_comb2

echo "[SUCCESS] Isolated C++20 Combination 2 compiled and linked."
$SCRATCH_DIR/app_cpp20_comb2
echo ""
echo "============================================================"
echo "             ALL INTEROP TESTS VERIFIED                     "


