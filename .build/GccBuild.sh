COMMON_FLAGS="-fverbose-asm -masm=intel -Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow \
    -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable"
OPTIMIZE_FLAGS="$COMMON_FLAGS -O2 -march=nehalem -mtune=native -flto -fno-math-errno -fno-trapping-math -s -Wl,--gc-sections"
PROFILE_FLAGS="$OPTIMIZE_FLAGS -fno-align-functions -fno-align-loops -fno-align-jumps -fno-align-labels"

gcc src/MakePrime.c $OPTIMIZE_FLAGS -obuild/Release/MakePrime.exe
./build/Release/makePrime src/Prime.c src/BuildPrime.c

if [ "$1" = "release" ]; then
    gcc src/BuildPrime.c $OPTIMIZE_FLAGS -obuild/Release/Prime.exe
    exit $?
elif [ "$1" = "profile" ]; then
    gcc src/BuildPrime.c -fprofile-generate $PROFILE_FLAGS -obuild/Release/Prime.exe

    build/Release/prime 10000000000000 10010000000000
    build/Release/prime 10010000000000 10020000000000
    build/Release/prime 10020000000000 10030000000000
    build/Release/prime 10030000000000 10040000000000
    build/Release/prime 10040000000000 10050000000000
    build/Release/prime 10050000000000 10060000000000
    build/Release/prime 10060000000000 10070000000000
    build/Release/prime 10070000000000 10080000000000
    build/Release/prime 10080000000000 10090000000000
    build/Release/prime 10090000000000 10100000000000

    gcc src/BuildPrime.c -fprofile-use -fprofile-correction $PROFILE_FLAGS -obuild/Release/Prime.exe
    rm build/Release/Prime-BuildPrime.gcda
    exit $?
elif [ "$1" = "asm" ]; then
    gcc src/BuildPrime.c -S $OPTIMIZE_FLAGS -fno-lto -obuild/Release/Prime.s
    exit $?
elif [ "$1" = "debug" ]; then
    gcc src/BuildPrime.c -g $COMMON_FLAGS -obuild/Debug/Prime.exe
    exit $?
else
    echo "Mode $1 is invalid [release, debug, asm, profile]"
    exit 1
fi