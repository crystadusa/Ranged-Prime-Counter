@echo off
set COMMON_FLAGS= /D_CRT_SECURE_NO_WARNINGS /Ipthread/include /nologo /permissive- /W4 /wd4245 pthread/lib/x64/pthreadVC2.lib
set OPTIMIZE_FLAGS= %COMMON_FLAGS% /O2

cl src/MakePrime.c /Fobuild/release/MakePrime.o %OPTIMIZE_FLAGS% /Febuild/release/MakePrime.exe
"build/release/MakePrime" src/Prime.c src/BuildPrime.c

if "%1"=="release" (
    copy pthread\dll\x64\pthreadVC2.dll build\release
    cl src/BuildPrime.c /Fobuild/release/Prime.o %OPTIMIZE_FLAGS% /Febuild/release/Prime.exe
    exit /b.
) else if "%1"=="profile" (
    copy pthread\dll\x64\pthreadVC2.dll build\release
    cl src/BuildPrime.c /GL /Fobuild/release/Prime.o %OPTIMIZE_FLAGS% /Febuild/release/Prime.exe /link /GENPROFILE /LTCG

    "build/release/prime" 00000000000 10000000000
    "build/release/prime" 10000000000 20000000000
    "build/release/prime" 20000000000 30000000000
    "build/release/prime" 30000000000 40000000000
    "build/release/prime" 40000000000 50000000000
    "build/release/prime" 50000000000 60000000000
    "build/release/prime" 60000000000 70000000000
    "build/release/prime" 70000000000 80000000000
    "build/release/prime" 80000000000 90000000000
    "build/release/prime" 90000000000 100000000000

    cl src/BuildPrime.c /GL /Fobuild/release/Prime.o %OPTIMIZE_FLAGS% /Febuild/release/Prime.exe /link /USEPROFILE /LTCG
    del build\release\prime.pgd build\release\*.pgc
) else if "%1"=="asm" (
    copy pthread\dll\x64\pthreadVC2.dll build\release
    cl src/BuildPrime.c /Fabuild/release/Prime.s /Fobuild/release/Prime.o %OPTIMIZE_FLAGS% /Febuild/release/Prime.exe
    exit /b
) else if "%1"=="debug" (
    copy pthread\dll\x64\pthreadVC2.dll build\debug
    cl src/BuildPrime.c /Zi /Fobuild/debug/Prime.o %COMMON_FLAGS% /Fdbuild/debug/Prime.pdb /Febuild/debug/Prime.exe
    exit /b
) else (
    echo Mode %1 is invalid [release, debug, asm]
    exit /b
)