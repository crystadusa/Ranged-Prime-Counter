COMMON_FLAGS="-fuse-ld=lld -fverbose-asm -masm=intel -Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow \
    -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable"
OPTIMIZE_FLAGS="$COMMON_FLAGS -O2 -march=nehalem -mtune=native -flto -fno-asynchronous-unwind-tables \
    -fno-math-errno -fno-trapping-math -s"

clang src/MakePrime.c $OPTIMIZE_FLAGS -obuild/Release/MakePrime.exe
./build/Release/makePrime src/Prime.c src/BuildPrime.c

if [ "$1" = "release" ]; then
    clang src/BuildPrime.c $OPTIMIZE_FLAGS -obuild/Release/Prime.exe
    exit $?
elif [ "$1" = "profile" ]; then
    clang src/BuildPrime.c -fprofile-generate $OPTIMIZE_FLAGS -obuild/Release/Prime.exe

    PROF_PREFIX="build/Prime-BuildPrime"
    export LLVM_PROFILE_FILE=$PROF_PREFIX"0.profraw"
    ./build/prime 00000000000 10000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"1.profraw"
    ./build/prime 10000000000 20000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"2.profraw"
    ./build/prime 20000000000 30000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"3.profraw"
    ./build/prime 30000000000 40000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"4.profraw"
    ./build/prime 40000000000 50000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"5.profraw"
    ./build/prime 50000000000 60000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"6.profraw"
    ./build/prime 60000000000 70000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"7.profraw"
    ./build/prime 70000000000 80000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"8.profraw"
    ./build/prime 80000000000 90000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"9.profraw"
    ./build/prime 90000000000 100000000000

    llvm-profdata merge build/*.profraw -o=$PROF_PREFIX.profdata
    clang src/BuildPrime.c -fprofile-use=$PROF_PREFIX.profdata $OPTIMIZE_FLAGS -obuild/Release/Prime.exe
    
    rm build/*.profraw
    rm $PROF_PREFIX.profdata
    exit $?
elif [ "$1" = "asm" ]; then
    clang src/BuildPrime.c -S $OPTIMIZE_FLAGS -fno-lto -obuild/Release/Prime.s
    exit $?
elif [ "$1" = "debug" ]; then
    clang src/BuildPrime.c -g $COMMON_FLAGS -obuild/Debug/Prime.exe
    exit $?
else
    echo "Mode $1 is invalid [release, debug, asm, profile]"
    exit 1
fi