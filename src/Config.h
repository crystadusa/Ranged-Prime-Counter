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
