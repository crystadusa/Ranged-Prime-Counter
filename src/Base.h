// New names for stdint types
#include <stdint.h>
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef float r32;
typedef int32_t s32;
typedef uint32_t u32;
typedef double r64;
typedef int64_t s64;
typedef uint64_t u64;

// Cross compiler extensions
#ifdef __GNUC__
#define FALLTHROUGH __attribute__((fallthrough));
#define UNREACHABLE __builtin_unreachable()
#define POPCOUNT_64(x) __builtin_popcountll(x)
#define AtomicAdd(dest, value) __atomic_fetch_add(dest, value, __ATOMIC_RELAXED)

#elif _MSC_VER
#define FALLTHROUGH
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
