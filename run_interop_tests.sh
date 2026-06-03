#!/bin/bash
set -eo pipefail


usage() {
  echo "Usage: $0 [options] [opt_level]"
  echo "Options:"
  echo "  --llvm-dir <path>   Path to llvm-project directory (default: .)"
  echo "  --gcc-dir <path>    Path to gcc-2.95 directory (default: <llvm-dir>/agent_scratch/gcc-2.95)"
  echo "  --clangxx <path>    Path to clang++ binary (default: <llvm-dir>/build/bin/clang++)"
  echo "                      (clang binary is assumed to be in the same directory)"
  exit 1
}

validate_file() {
  if [ ! -f "$1" ]; then
    echo "Error: File $1 does not exist." >&2
    exit 1
  fi
  if [ ! -x "$1" ]; then
    echo "Error: File $1 is not executable." >&2
    exit 1
  fi
}

validate_dir() {
  if [ ! -d "$1" ]; then
    echo "Error: Directory $1 does not exist." >&2
    exit 1
  fi
}

validate_command() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Error: Required command '$1' not found." >&2
    exit 1
  fi
}

LLVM_PROJECT_DIR="."
GCC_DIR=""
CLANGXX=""

# Parse arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --llvm-dir)
      LLVM_PROJECT_DIR="$2"
      shift 2
      ;;
    --gcc-dir)
      GCC_DIR="$2"
      shift 2
      ;;
    --clangxx)
      CLANGXX="$2"
      shift 2
      ;;
    -h|--help)
      usage
      ;;
    -O*)
      OPT_LEVEL="$1"
      shift
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      ;;
    *)
      break
      ;;
  esac
done

OPT_LEVEL="${OPT_LEVEL:-${1:--O0}}"

# Default paths if not specified
if [ -z "$GCC_DIR" ]; then
  GCC_DIR="$LLVM_PROJECT_DIR/agent_scratch/gcc-2.95"
fi

if [ -z "$CLANGXX" ]; then
  CLANGXX="$LLVM_PROJECT_DIR/build/bin/clang++"
fi

# Derive CLANG from CLANGXX
if [[ "$CLANGXX" == /* ]] || [[ "$CLANGXX" == ./* ]] || [[ "$CLANGXX" == ../* ]]; then
  CLANG_DIR=$(dirname "$CLANGXX")
  CLANG="$CLANG_DIR/clang"
else
  CLANG=$(which clang || echo "clang")
fi

# Validation
echo "Validating paths..."
validate_dir "$LLVM_PROJECT_DIR"
validate_dir "$GCC_DIR"
validate_file "$CLANGXX"
validate_file "$CLANG"
validate_command nm

GPP_BIN="$GCC_DIR/usr/bin/g++-2.95"
validate_file "$GPP_BIN"

GCC_LIB_DIR="$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4"
validate_dir "$GCC_LIB_DIR"

SCRATCH_DIR="$LLVM_PROJECT_DIR/agent_scratch/gcc2_interop_final"

CLANGPP="$CLANGXX --target=i386-pc-linux-gnu -fc++-abi=gcc2 -nostartfiles -nodefaultlibs -Wl,--no-eh-frame-hdr -fexceptions -fcxx-exceptions -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/Scrt1.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crti.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtbeginS.o"
GPP="$GPP_BIN -Wa,--32 -B$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/ $OPT_LEVEL -fpermissive -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"
CLANG_CC1="$CLANG -cc1 -triple i386-pc-linux-gnu -fc++-abi=gcc2 -std=c++98 -fexceptions -fcxx-exceptions $OPT_LEVEL -D__extern_always_inline=__inline__ -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"
CLANG_CC1_CXX20="$CLANG -cc1 -triple i386-pc-linux-gnu -fc++-abi=gcc2 -std=c++20 -fno-sized-deallocation -Wno-dynamic-exception-spec -fexceptions -fcxx-exceptions $OPT_LEVEL -D__extern_always_inline=__inline__ -I$GCC_DIR/usr/include/g++-3 -I$GCC_DIR/usr/include -I$GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/include -I/usr/include"


echo "============================================================"
echo "   LEGACY GCC 2.95 & CLANG GCC2 ABI INTEROP VERIFICATION    "
echo "   Optimization Level: $OPT_LEVEL                           "
echo "============================================================"
echo "Cleaning up stale build artifacts..."
rm -f $SCRATCH_DIR/app_comb* $SCRATCH_DIR/app_cpp20_comb* $SCRATCH_DIR/*.o $SCRATCH_DIR/*.ll
mkdir -p $SCRATCH_DIR



echo ""
echo "------------------------------------------------------------"
echo " COMBINATION 1: Build LIB with Clang, CONSUMER with GCC 2.95"
echo "------------------------------------------------------------"

$CLANG_CC1 -emit-obj lib.cpp -o $SCRATCH_DIR/lib_clang.o

$GPP -c consumer.cpp -o $SCRATCH_DIR/consumer_gcc.o

$CLANGPP $SCRATCH_DIR/lib_clang.o $SCRATCH_DIR/consumer_gcc.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_comb1

echo "[SUCCESS] Combination 1 compiled and linked successfully."
echo "Running Combination 1:"
$SCRATCH_DIR/app_comb1

echo ""
echo "------------------------------------------------------------"
echo " COMBINATION 2: Build LIB with GCC 2.95, CONSUMER with Clang"
echo "------------------------------------------------------------"

$GPP -c lib.cpp -o $SCRATCH_DIR/lib_gcc.o

$CLANG_CC1 -emit-obj consumer.cpp -o $SCRATCH_DIR/consumer_clang.o

echo "Verifying local class mangling inside explicit template specialization..."
if ! nm $SCRATCH_DIR/consumer_clang.o | grep "get_val__Q235test_spec_local_class__H1Zi_X01_i.0_5Local" >/dev/null; then
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
$CLANG_CC1_CXX20 -emit-obj lib_cpp20.cpp -o $SCRATCH_DIR/lib_cpp20_clang.o
$GPP -c consumer_cpp20.cpp -o $SCRATCH_DIR/consumer_cpp20_gcc.o

$CLANGPP $SCRATCH_DIR/lib_cpp20_clang.o $SCRATCH_DIR/consumer_cpp20_gcc.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_cpp20_comb1

echo "[SUCCESS] Isolated C++20 Combination 1 compiled and linked."
$SCRATCH_DIR/app_cpp20_comb1

# Option B: Lib in GCC 2.95 (C++98), Consumer in Clang (C++20)
$GPP -c lib_cpp20.cpp -o $SCRATCH_DIR/lib_cpp20_gcc.o
$CLANG_CC1_CXX20 -emit-obj consumer_cpp20.cpp -o $SCRATCH_DIR/consumer_cpp20_clang.o

$CLANGPP $SCRATCH_DIR/lib_cpp20_gcc.o $SCRATCH_DIR/consumer_cpp20_clang.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_cpp20_comb2

echo "[SUCCESS] Isolated C++20 Combination 2 compiled and linked."
$SCRATCH_DIR/app_cpp20_comb2

echo ""
echo "------------------------------------------------------------"
echo " COMBINATION 3: Build BOTH Lib and Consumer with Clang"
echo "------------------------------------------------------------"

$CLANG_CC1 -emit-obj lib.cpp -o $SCRATCH_DIR/lib_clang_only.o
$CLANG_CC1 -emit-obj consumer.cpp -o $SCRATCH_DIR/consumer_clang_only.o

$CLANGPP $SCRATCH_DIR/lib_clang_only.o $SCRATCH_DIR/consumer_clang_only.o $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libstdc++.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/libgcc.a $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtendS.o -lc $GCC_DIR/usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtn.o -o $SCRATCH_DIR/app_comb3

echo "[SUCCESS] Combination 3 compiled and linked successfully."
echo "Running Combination 3:"
$SCRATCH_DIR/app_comb3

echo ""
echo "============================================================"
echo "             ALL INTEROP TESTS VERIFIED                     "
