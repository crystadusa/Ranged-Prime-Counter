#include "Base.h"
#include "Config.h"

// Includes
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Memory copy with destination offset
#define strCpyOffset(dest, offset, src)\
    memcpy(dest + offset, src, sizeof src - 1);\
    offset += sizeof src - 1

// Struct definitions
typedef struct {
    u64 searchStart;
    u64 searchEnd;
    u32 l1CacheSize;
} primeSieveArgs;

typedef struct {
    u32 wheel;
    u32 factor;
    u64 multiple;
} prime;

// Static lookup tables
static u8 primeRemainderToBit [] = {7, 7, 7, 0, 0, 1, 2, 2, 3, 4, 4, 5, 5, 5, 6};
static u8 bitToPrimeRemainder [] = {7, 11, 13, 2, 4, 8, 14, 1};
static u8 nextWheelMultiple [] = {0, 4, 2, 0, 2, 0, 0, 2, 0, 0, 2, 0, 4, 2, 0};
static u8 startInvalidBits [] = {7, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 6};
static u8 endInvalidBits [] = {0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 6, 7};

/* Unused static data for sieving large prime factors
static u8 unprimeBit [] = {
    ~16, ~8, (u8)~128, ~64, ~4, ~2, ~32, ~1,
    ~8, (u8)~128, ~32, ~1, ~64, ~4, ~16, ~2,
    (u8)~128, ~32, ~16, ~2, ~1, ~64, ~8, ~4,
    ~64, ~1, ~2, ~16, ~32, (u8)~128, ~4, ~8,
    ~4, ~64, ~1, ~32, (u8)~128, ~8, ~2, ~16,
    ~2, ~4, ~64, (u8)~128, ~8, ~16, ~1, ~32,
    ~32, ~16, ~8, ~4, ~2, ~1, (u8)~128, ~64,
    ~1, ~2, ~4, ~8, ~16, ~32, ~64, (u8)~128,
};

static u8 multipleDist [] = {2, 1, 2, 1, 2, 3, 1, 3};

static u8 multipleCorrection [] = {
    1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 2, 0, 2, 2, 1, 2,
    2, 1, 2, 1, 1, 3, 1, 2,
    1, 0, 0, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 1, 1, 0, 1,
    1, 0, 1, 1, 1, 2, 0, 2,
    2, 1, 2, 1, 2, 2, 1, 3,
    0, 0, 0, 0, 0, 0, 0, 1,
};

static u32 nextWheel [] = {
    1, 2, 3, 4, 5, 6, 7, 0,
    9, 10, 11, 12, 13, 14, 15, 8,
    17, 18, 19, 20, 21, 22, 23, 16,
    25, 26, 27, 28, 29, 30, 31, 24,
    33, 34, 35, 36, 37, 38, 39, 32,
    41, 42, 43, 44, 45, 46, 47, 40,
    49, 50, 51, 52, 53, 54, 55, 48,
    57, 58, 59, 60, 61, 62, 63, 56
};
*/

// Function declarations
int main(int, char* []);
static u32 strAppendInt(u8*, u64);
static u64 sievePrimes(primeSieveArgs*);
static void sieveSmallPrimes(prime*, u32, u8*, u64, u32, u64);
static void sieveMediumPrimes(prime*, u32*, prime*, u32*, u32, u8*, u64, u64);
