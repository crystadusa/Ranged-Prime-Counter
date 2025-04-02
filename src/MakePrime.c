#include "ctype.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef int8_t s8;
typedef uint8_t u8;
typedef float r32;
typedef int32_t s32;
typedef uint32_t u32;
typedef double r64;
typedef int64_t s64;
typedef uint64_t u64;

#define NEW_LINE() \n
#define _STRINGIFY(macro) #macro
#define STRINGIFY(macro) _STRINGIFY(macro)

// sieveSmallPrimeFactor() format string replacement
#define multipleRemLoop() \
\n%0\t\tisFoundPrime[subPrimeMultiple + subPrimeFactor * %5 + %6] &= ~%7;%11

#define multipleRemNoCase() \
%0\tif (subPrimeMultiple > lastSmallIndex) {\n\
%0\t\tprimes[k].wheel = %3;\n\
%0\t\tgoto nextSmallPrimeFactor;\n\
%0\t}\n\
%0\tisFoundPrime[subPrimeMultiple] &= ~%7;\n\
%0\tsubPrimeMultiple += subPrimeFactor * %9 + %10;\n%11

#define multipleRemCase() \
\n%0\t%2\n\
%0\tFALLTHROUGH case %3:NEW_LINE()\
multipleRemNoCase()

#define sieveSmallPrimeFactor \
%0%1\n\
%0while (1) {\n\
%0\t%2\n\
%0\tcase %3: while ((s64) subPrimeMultiple <= (s64) lastSmallIndex - (s64) subPrimeFactor * 12 - %4) {\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\
multipleRemLoop()\n%12\
%0\t\tsubPrimeMultiple += subPrimeFactor * 15 + %8;\n\
%0\t}NEW_LINE()\
multipleRemNoCase()\
multipleRemCase()\
multipleRemCase()\
multipleRemCase()\
multipleRemCase()\
multipleRemCase()\
multipleRemCase()\
multipleRemCase()\
%0}\n

// sieveMediumPrimeFactor() format string replacement
#define multipleRemNoCase2() \
%0\tif (subPrimeMultiple > sectionSize) {\n\
%0\t\twheel = %3;\n\
%0\t\tgoto nextLargePrimeFactor;\n\
%0\t}\n\
%0\tisFoundPrime[subPrimeMultiple] &= ~%7;\n\
%0\tsubPrimeMultiple += subPrimeFactor * %9 + %10;\n%11

#define multipleRemCase2() \
\n%0\t%2\n\
%0\tFALLTHROUGH case %3:NEW_LINE()\
multipleRemNoCase2()

#define sieveMediumPrimeFactor \
%0%1\n\
%0while (1) {\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
multipleRemCase2()\
%0}\n\

static void sievePrimePrintFormat(FILE* destFile, char* argumentStr, char* sieveStr, char* sieveFormatStr, s32 sieveFormatSize) {
    // Reads numerical argument in parenthesis and returns if not found
    if (argumentStr[0] != '(') {
        printf("Expected '(' after \"%s.\"\n", sieveStr);
        return;
    }

    s32 digitOffset = 1;
    while (isdigit(argumentStr[digitOffset])) digitOffset++;
    if (digitOffset == 1) {
        printf("Expected number after \"%s(.\"\n", sieveStr);
        return;
    }

    if (argumentStr[digitOffset] != ')') {
        printf("Expected ')' after \"%s(...\"\n", sieveStr);
        return;
    }

    // Copies formatted string to destination file
    s32 factorRemainder = atol(argumentStr + 1);
    s32 sieveFormatIndex = 0;
    s32 multipleRemainderIndex = 0;
    for (s32 j = 0; j < sieveFormatSize; j++) {
        if (sieveFormatStr[j] != '%') continue;

        if(j != 0 && sieveFormatStr[j - 1] == '\\') {
            // Copies all characters before '\%' and writes '%'
            fwrite(sieveFormatStr + sieveFormatIndex, 1, j - sieveFormatIndex - 1, destFile);
            fputs("%", destFile);
            sieveFormatIndex = j + 1;
            continue;
        }

        // Copies all characters before '%'
        fwrite(sieveFormatStr + sieveFormatIndex, 1, j - sieveFormatIndex, destFile);

        // Reads numerical argument in the formatted string
        s32 digitOffset1 = 1;
        while (isdigit(sieveFormatStr[j + digitOffset1])) digitOffset1++;
        if (digitOffset1 == 1) {
            puts("Expected number after \"%.\"");
            break;
        }

        s32 argumentPos = atol(sieveFormatStr + j + 1);
        j += digitOffset1 - 1;

        static s8 primeRemainderToIndex [] = {7, 7, 7, 0, 0, 1, 2, 2, 3, 4, 4, 5, 5, 5, 6};
        static s8 primeIndexToRemainder [] = {7, 11, 13, 17, 19, 23, 29, 31, 37};
        s32 multipleRemainder = primeIndexToRemainder[multipleRemainderIndex];
        s32 primeIndexDistance = primeIndexToRemainder[multipleRemainderIndex + 1] - multipleRemainder;

        // Replaces %s in formatted string based on numerical argument
        switch (argumentPos) {
            case 0: {
                fputs("\t\t\t\t", destFile);
                break;
            }
            case 1: {
                fprintf(destFile, "// Prime factor %% 30 == %d", factorRemainder % 30);
                break;
            }
            case 2: {
                fprintf(destFile, "// Prime multiple / %d %% 30 == %d", factorRemainder % 30, multipleRemainder % 30);
                break;
            }
            case 3: {
                fprintf(destFile, "%d", primeRemainderToIndex[factorRemainder / 2] * 8 + multipleRemainderIndex);
                break;
            }
            case 4: {
                fprintf(destFile, "%d", ((factorRemainder * 7 - 2) % 30 + factorRemainder % 15 * 24) / 30);
                break;
            }
            case 5: {
                fprintf(destFile, "%d", multipleRemainder / 2 - 3);
                break;
            }
            case 6: {
                fprintf(destFile, "%d", ((factorRemainder * 7 - 2) % 30 + factorRemainder % 15 * (multipleRemainder - 7)) / 30);
                break;
            }
            case 7: {
                fprintf(destFile, "%d", (u8) (1 << primeRemainderToIndex[factorRemainder * multipleRemainder / 2 % 15]));
                break;
            }
            case 8: {
                fprintf(destFile, "%d", factorRemainder % 15);
                break;
            }
            case 9: {
                fprintf(destFile, "%d", primeIndexDistance / 2);
                break;
            }
            case 10: {
                fprintf(destFile, "%d", ((factorRemainder * multipleRemainder - 2) % 30 + factorRemainder % 15 * primeIndexDistance) / 30);
                break;
            }
            case 11: {
                multipleRemainderIndex++;
                break;
            }
            case 12: {
                multipleRemainderIndex = 0;
                break;
            }
            default: {
                printf("sievePrimeFactor argument %d not handled.\n", argumentPos);
                break;
            };
        }

        sieveFormatIndex = j + 1;
    }

    // Copies the rest of the formatted string
    fwrite(sieveFormatStr + sieveFormatIndex, 1, sieveFormatSize - sieveFormatIndex, destFile);
}

int main(int argCount, char* argArray []) {
    // Opens source and destination files
    if (argCount < 3) {
        puts("No source or destination files found.");
        return 1;
    }

    FILE* srcFile = fopen(argArray[1], "rb");
    if (!srcFile) {
        printf("Failed to read the file \"%s\".", argArray[1]);
        return 1;
    }

    FILE* destFile = fopen(argArray[2], "wb");
    if (!destFile) {
        printf("Failed to create the file \"%s\".", argArray[2]);
        return 1;
    }

    static char sieveSmallStr [] = "sieveSmallPrimeFactor";
    static char sieveMediumStr [] = "sieveMediumPrimeFactor";
    static char sieveSmallFormatStr [] = STRINGIFY(sieveSmallPrimeFactor);
    static char sieveMediumFormatStr [] = STRINGIFY(sieveMediumPrimeFactor);

    // Copies from the source to destination file a line at a time
    char fileBuffer [64];
    while (fgets(fileBuffer, 64, srcFile)) {
        // Skips white space characters at the begining of a line
        s32 whiteSpaceOffset = 0;
        while(isblank(fileBuffer[whiteSpaceOffset])) whiteSpaceOffset++;
    
        // Search and replace of certain character sequences with arguments
        if (strncmp(fileBuffer + whiteSpaceOffset, sieveSmallStr, sizeof(sieveSmallStr) - 1) == 0)
            sievePrimePrintFormat(destFile, fileBuffer + whiteSpaceOffset + sizeof(sieveSmallStr) - 1, 
                sieveSmallStr, sieveSmallFormatStr, sizeof(sieveSmallFormatStr) - 1);
                
        else if (strncmp(fileBuffer + whiteSpaceOffset, sieveMediumStr, sizeof(sieveMediumStr) - 1) == 0)
            sievePrimePrintFormat(destFile, fileBuffer + whiteSpaceOffset + sizeof(sieveMediumStr) - 1, 
                sieveMediumStr, sieveMediumFormatStr, sizeof(sieveMediumFormatStr) - 1);

        else fputs(fileBuffer, destFile);
    }
}
