# Crystadusa's ranged prime counter 
### About
This project is my attempt at creating an optimal binary to accurately calculate the number of prime numbers in an arbitrary range. There is definite room for polish, but I consider this project relatively performant regardless. Many implementation details were inspired by [this repository](https://github.com/kimwalisch/primesieve.git). It uses several algorithms like a [segmented](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes#Segmented_sieve) [sieve of eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes), increases L1 data cache hits by using a smaller segment size for small prime factors, bit arrays with a [mod 30 wheel](https://en.wikipedia.org/wiki/Wheel_factorization), multithreading, and reduced branch misprediction by sorting medium primes factors.

### Command line syntax
Prime \<End\> \<Options\>  
Prime \<Start\> \<End\> \<Options\>

Options:
* -t\<Threads\>

### Configuration
I recommend tweaking some defines in Prime.h before compilation.
1. set MAX_THREAD_COUNT to your cpu's thread count
2. set SMALL_SECTION_SIZE to a few KBs less than your CPU's L1 data cache
3. The rest can be tweaked for per processor performance.

### Build
This project has shell scripts for gcc, clang, and msvc in addition to the cmake build system.

To build this project with unix makefiles enter the following commands in a new build directory.
1. cmake -S.. -B. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=[RELEASE, DEBUG]
2. cmake --build .

To build this project on windows enter the following commands in the "x86_x64 Cross Tools Command Prompt for VS 2022" and a new build directory.
1. cmake -S.. -B. -G"Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=[RELEASE, DEBUG]
2. cmake --build . --config [Release, Debug]

-DCMAKE_BUILD_TYPE=[GENPROFILE or USEPROFILE] are used to enable profile guided optimizations. Run with genprofile and then useprofile for supported compilers.
"# Ranged-Prime-Counter" 
