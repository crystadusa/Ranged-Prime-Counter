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

    PROF_PREFIX="build/Release/Prime-BuildPrime"
    export LLVM_PROFILE_FILE=$PROF_PREFIX"0.profraw"
    build/Release/prime 10000000000000 10010000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"1.profraw"
    build/Release/prime 10010000000000 10020000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"2.profraw"
    build/Release/prime 10020000000000 10030000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"3.profraw"
    build/Release/prime 10030000000000 10040000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"4.profraw"
    build/Release/prime 10040000000000 10050000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"5.profraw"
    build/Release/prime 10050000000000 10060000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"6.profraw"
    build/Release/prime 10060000000000 10070000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"7.profraw"
    build/Release/prime 10070000000000 10080000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"8.profraw"
    build/Release/prime 10080000000000 10090000000000
    export LLVM_PROFILE_FILE=$PROF_PREFIX"9.profraw"
    build/Release/prime 10090000000000 10100000000000

    llvm-profdata merge build/Release/*.profraw -o=$PROF_PREFIX.profdata
    clang src/BuildPrime.c -fprofile-use=$PROF_PREFIX.profdata $OPTIMIZE_FLAGS -obuild/Release/Prime.exe
    
    rm build/Release/*.profraw
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