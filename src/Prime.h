// Cross platform compiler extensions
#ifdef __GNUC__
#define FALLTHROUGH __attribute__((fallthrough));
#define UNREACHABLE __builtin_unreachable()
#define POPCOUNT_64(x) __builtin_popcountll(x)
#define AtomicAdd(dest, value) __atomic_fetch_add(dest, value, __ATOMIC_RELAXED)

#elif _MSC_VER
#define FALLTHROUGH ;
#define UNREACHABLE __assume(0)
#define POPCOUNT_64(x) __popcnt64(x)
#define AtomicAdd(dest, value) _InlineInterlockedAdd64((volatile LONG64*) dest, value)

#else
#error Compiler Not Supported!
#endif

// Cross platform threading layer
#ifndef _WIN32
#include <pthread.h>

typedef pthread_t thread;
static void threadCreate(thread* threadP, void*(*func)(void*), void* args) {
    pthread_create(threadP, 0, (void*(*)(void*)) func, args);
}

static void* threadJoin(thread* threadP) {
    void* ret;
    pthread_join(*threadP, &ret);
    return ret;
}

#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
    HANDLE thread;
    void*(*func)(void*);
    void* args;
    void* ret;
} thread;

static void threadWrapper(thread* threadP) {
    threadP->ret = threadP->func(threadP->args);
}

static void threadCreate(thread* threadP, void*(*func)(void*), void* args) {
    threadP->func = func;
    threadP->args = args;
    threadP->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) threadWrapper, threadP, 0, 0);
}

static void* threadJoin(thread* threadP) {
    WaitForSingleObject(threadP->thread, INFINITE);
    return threadP->ret;
}
#endif

// Configuration defines
#define SMALL_SECTION_SIZE (40 * 1024)
#if SMALL_SECTION_SIZE && SMALL_SECTION_SIZE % 16
#error SMALL_SECTION_SIZE must be a multiple of 16
#endif

#define MAX_SECTION_SIZE (240 * 1024)
#if MAX_SECTION_SIZE < 1
#error MAX_SECTION_SIZE must be at least 1
#endif

#define MIN_THREAD_SIZE 10000000
#if MIN_THREAD_SIZE < 1
#error MIN_THREAD_SIZE must be at least 1
#endif

#define MAX_THREAD_COUNT 64
#if MAX_THREAD_COUNT < 1
#error MAX_THREAD_COUNT must be at least 1
#endif

// Medium primes are >= small section size / x
#define MEDIUM_PRIME_FACTOR 120
// Large primes are >= MAX_SECTION_SIZE * 30 / x
#define LARGE_PRIME_FACTOR 10

#if MEDIUM_PRIME_FACTOR < LARGE_PRIME_FACTOR
#error MEDIUM_PRIME_FACTOR must be greater than LARGE_PRIME_FACTOR
#endif

// Section size is >= x * sqrt(search end)
#define SECTION_SIZE_FACTOR 30
#if SECTION_SIZE_FACTOR < 1
#error SECTION_SIZE_FACTOR must be at least 1
#endif

// Includes
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// New names for stdint types
typedef int8_t s8;
typedef uint8_t u8;
typedef float r32;
typedef int32_t s32;
typedef uint32_t u32;
typedef double r64;
typedef int64_t s64;
typedef uint64_t u64;

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
