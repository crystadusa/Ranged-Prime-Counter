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

    "build/Release/prime" 10000000000000 10010000000000
    "build/Release/prime" 10010000000000 10020000000000
    "build/Release/prime" 10020000000000 10030000000000
    "build/Release/prime" 10030000000000 10040000000000
    "build/Release/prime" 10040000000000 10050000000000
    "build/Release/prime" 10050000000000 10060000000000
    "build/Release/prime" 10060000000000 10070000000000
    "build/Release/prime" 10070000000000 10080000000000
    "build/Release/prime" 10080000000000 10090000000000
    "build/Release/prime" 10090000000000 10100000000000

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