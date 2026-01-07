@echo off
set COMMON_FLAGS= /D_CRT_SECURE_NO_WARNINGS /nologo /permissive- /W4 /wd4245
set OPTIMIZE_FLAGS= %COMMON_FLAGS% /O2 /Ot /MD

cl src/MakePrime.c /Fobuild/Release/MakePrime.o %OPTIMIZE_FLAGS% /Febuild/Release/MakePrime.exe
"build/Release/MakePrime" src/Prime.c src/BuildPrime.c

if "%1"=="release" (
    cl src/BuildPrime.c /Fobuild/Release/Prime.o %OPTIMIZE_FLAGS% /Febuild/Release/Prime.exe
    exit /b
) else if "%1"=="profile" (
    cl src/BuildPrime.c /GL /Fobuild/Release/Prime.o %OPTIMIZE_FLAGS% /Febuild/Release/Prime.exe /link /GENPROFILE /LTCG

    "build/Release/prime" 00000000000 10000000000
    "build/Release/prime" 10000000000 20000000000
    "build/Release/prime" 20000000000 30000000000
    "build/Release/prime" 30000000000 40000000000
    "build/Release/prime" 40000000000 50000000000
    "build/Release/prime" 50000000000 60000000000
    "build/Release/prime" 60000000000 70000000000
    "build/Release/prime" 70000000000 80000000000
    "build/Release/prime" 80000000000 90000000000
    "build/Release/prime" 90000000000 100000000000

    cl src/BuildPrime.c /GL /Fobuild/Release/Prime.o %OPTIMIZE_FLAGS% /Febuild/Release/Prime.exe /link /USEPROFILE /LTCG
    del build\Release\prime.pgd build\Release\*.pgc
) else if "%1"=="asm" (
    cl src/BuildPrime.c /Fabuild/Release/Prime.s /Fobuild/Release/Prime.o %OPTIMIZE_FLAGS% /Febuild/Release/Prime.exe
    exit /b
) else if "%1"=="debug" (
    cl src/BuildPrime.c /Zi /Fobuild/Debug/Prime.o %COMMON_FLAGS% /MDd /Fdbuild/Debug/Prime.pdb /Febuild/Debug/Prime.exe
    exit /b
) else (
    echo Mode %1 is invalid [release, debug, asm, profile]
    exit /b
)