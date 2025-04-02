#include "Prime.h"

int main(int argCount, char* argArray []) {
    if (argCount < 2) {
        puts("No command line arguments found!\nPrime <End> <Options>\nPrime <Start> <End> <Options>\nOptions:\n  -t<Threads>");
        return 1;
    }

    // Evaluates sieve options from command line arguments
    u64 searchStart = 0;
    u64 searchEnd = 0;
    u32 maxThreadCount = 8;
    u32 numbersEntered = 0;

    for (u32 i = 1; i < (u32) argCount; i++) {
        // Checks for thread count option and returns an error for non positive numbers
        if (argArray[i][0] == '-' && argArray[i][1] =='t') {
            if (isdigit(argArray[i][2]) == 0) {
                puts("Invalid thread count!");
                return 1;
            }
            maxThreadCount = (u32) atoll(argArray[i] + 2);
            continue;
        }

        // Returns an error for arguments with a negative number or invalid option
        if (isdigit(argArray[i][0]) == 0) {
            if (isdigit(argArray[i][1])) puts("Negative numbers are invalid!");
            else puts("Invalid argument!");
            return 1;
        }

        // Sets search start and end depending on the number of arguments
        if (numbersEntered == 0) {
            searchEnd = atoll(argArray[i]);
            numbersEntered++;
        } else if (numbersEntered == 1) {
            searchStart = searchEnd;
            searchEnd = atoll(argArray[i]);
            numbersEntered++;
        }
    }

    if (numbersEntered == 0) {
        puts("No search end found!");
        return 1;
    }

    if (maxThreadCount > MAX_THREAD_COUNT) maxThreadCount = MAX_THREAD_COUNT;

    puts("Searching...");
    clock_t startTime = clock();

    // Calculates the size of each thread
    u64 searchSize = 0;
    u64 threadCount = 1;
    if (searchStart <= searchEnd) {
        searchSize = searchEnd - searchStart + 1;
        threadCount = searchSize / MIN_THREAD_SIZE;
        if (threadCount == 0) threadCount = 1;
        else if (threadCount > maxThreadCount) threadCount = maxThreadCount;
    }
    u64 threadSize = (searchSize - 1) / threadCount + 1;

    u32 l1CacheSize = SMALL_SECTION_SIZE;
    if (threadCount > 4) l1CacheSize /= 2;

    // Multithreaded prime sieve
    u64 primeCount = 0;
    if (threadCount < 2) {
        primeSieveArgs args = {searchStart, searchEnd, l1CacheSize};
        primeCount = sievePrimes(&args);
    } else {
        // Evenly distributes search range between threads
        pthread_t sieveThreads [8];
        primeSieveArgs args [8];
        for (u32 i = 0; i < threadCount; i++) {
            u64 threadStart = searchStart + threadSize * i;
            u64 threadEnd = threadStart + threadSize - 1;
            if (threadEnd > searchEnd) threadEnd = searchEnd;

            args[i] = (primeSieveArgs) {threadStart, threadEnd, l1CacheSize};
            pthread_create(&sieveThreads[i], 0, (void*(*)(void*)) sievePrimes, &args[i]);
        }

        // Adds together the prime counts from all threads
        for (u32 i = 0; i < threadCount; i++) {
            u64 threadPrimeCount;
            pthread_join(sieveThreads[i], (void**) &threadPrimeCount);
            primeCount += threadPrimeCount;
        }
    }

    clock_t endTime = clock();
    r32 elapsedTime = (r32) (endTime - startTime) / CLOCKS_PER_SEC;

    char resultStr [136];
    u32 resultI = 0;

    // Displays elapsed time
    if (elapsedTime > 999999999.0f) elapsedTime = 999999999.0f;
    resultI += strAppendInt((u8*) resultStr + resultI, (u32) elapsedTime);
    resultStr[resultI++] = '.';

    u32 elapsedTimeMS = (u32) ((elapsedTime - (r32) (u32) elapsedTime) * 1000);
    if (elapsedTimeMS < 100) resultStr[resultI++] = '0';
    if (elapsedTimeMS < 10) resultStr[resultI++] = '0';
    resultI += strAppendInt((u8*) resultStr + resultI, elapsedTimeMS);

    strCpyOffset(resultStr, resultI, " seconds have elapsed.\nThere are ");

    // Displays prime number count and range
    resultI += strAppendInt((u8*) resultStr + resultI, primeCount);

    if (searchStart == 0) {
        strCpyOffset(resultStr, resultI, " prime numbers up to ");
        resultI += strAppendInt((u8*) resultStr + resultI, searchEnd);
    } else {
        strCpyOffset(resultStr, resultI, " prime numbers between ");
        resultI += strAppendInt((u8*) resultStr + resultI, searchStart);
        strCpyOffset(resultStr, resultI, " and ");
        resultI += strAppendInt((u8*) resultStr + resultI, searchEnd);
    }

    resultStr[resultI] = '.';
    resultStr[resultI + 1] = '\0';

    puts(resultStr);
    return 0;
}

static u32 strAppendInt(u8* str, u64 toStr) {
    u32 digits = 1;
    for (u64 i = toStr; i > 9; i /= 10) digits++;
    for (u32 i = digits; i;) {
        str[--i] = (u8) ('0' + toStr % 10);
        toStr /= 10;
    }
    return digits;
}

static u64 sievePrimes(primeSieveArgs* args) {
    u64 searchStart = args->searchStart < 2 ? 2 : args->searchStart;
    u64 byteStart = (searchStart - 2) / 30;

    u64 searchEnd = args->searchEnd;

    // Corrects error of not counting 2, 3, and 5 as prime
    if (searchStart > searchEnd) return 0;
    if (searchEnd < 5) {
        u64 foundPrimeCount = 2;
        if (searchStart > 3 || searchEnd < 2) foundPrimeCount = 0;
        else if (searchStart == 3 || searchEnd == 2) foundPrimeCount = 1;
        return foundPrimeCount;
    }

    u64 byteEnd = searchEnd < 7 ? 7 : searchEnd;
    u32 sqrtSearchCap = (u32) sqrt((r64) byteEnd);
    byteEnd = (u64) (byteEnd - 7) / 30;

    u32 l1CacheSize = args->l1CacheSize * 30;

    // Marks non primes up to the square root of the search cap
    u8* isPrimeFactor = calloc(sqrtSearchCap + 1, 1);
    for (u32 i = 3; i * i <= sqrtSearchCap; i++)
        for (u32 j = i * i; j <= sqrtSearchCap; j += i)
            isPrimeFactor[j] = 1;

    /*
    https://doi.org/10.48550/arXiv.1002.0442

    p(x) = the number of primes up to x
    l(x) = x / (ln(x) - 1.1)

    Remark 6.5 proves l(x) >= p(x) for x >= 60184
    I have tested that l(x) + 2 >= p(x) - 3 for x >= 4

    Table 6.8 shows smaller constants work as narrower bounds for larger x values
    however 1.1 is acceptable for purposes of reducing memory allocation
    */
    u32 primeFactorAprx = sqrtSearchCap > 3 ? (u32) ((r32) sqrtSearchCap / (r32) (log(sqrtSearchCap) - 1.1)) + 2 : 0;

    prime* primes = calloc(primeFactorAprx, sizeof(prime));

    // Different counts based on prime factor size
    u32 sectionSizeCap = MAX_SECTION_SIZE * 30;
    u32 smallFactorCount = 0;
    u32 mediumFactorCount = 0;
    u32 primeFactorCount = 0;

    // Adds unmarked numbers to the prime count
    for (u32 i = 7; i <= sqrtSearchCap; i += 2)
        if (isPrimeFactor[i] == 0) {
            u64 multiple = (searchStart - 1) / i + 1;
            if (multiple < i) multiple = i;

            if (multiple % 2 == 0) multiple++;
            multiple += nextWheelMultiple[multiple % 30 / 2];

            primes[primeFactorCount].factor = i / 15;
            primes[primeFactorCount].multiple = (multiple * i - 2) / 30;
            primes[primeFactorCount++].wheel = primeRemainderToBit[i % 30 / 2] * 8 + primeRemainderToBit[multiple % 30 / 2];

            if (i * MEDIUM_PRIME_FACTOR < l1CacheSize) smallFactorCount++;
            // if (i * LARGE_PRIME_FACTOR < sectionSizeCap) 
            mediumFactorCount++;
        }

    // Clips the section size between l1 cache size and MAX_SECTION_SIZE
    u32 maxSectionSize = sqrtSearchCap * SECTION_SIZE_FACTOR;
    if (mediumFactorCount && maxSectionSize < l1CacheSize * 2) maxSectionSize = l1CacheSize * 2;
    if (maxSectionSize > sectionSizeCap) maxSectionSize = sectionSizeCap;
    maxSectionSize = ((maxSectionSize - 1) / l1CacheSize + 1) * l1CacheSize;
    l1CacheSize /= 30;
    maxSectionSize /= 30;

    // Allocates memory for sectioned bit arrays
    free(isPrimeFactor);
    u8* isFoundPrime = malloc(maxSectionSize);
    u64 foundPrimeCount = 0;

    // Initialises sorted prime arrays for medium primes
    u32 sortedPrimeSize = primeFactorAprx / 32; // TODO (formal proof this memory reduction is allowed)
    prime* sortedPrimesSrc = calloc(sortedPrimeSize * 64, sizeof(prime));
    prime* sortedPrimesDest = calloc(sortedPrimeSize * 64, sizeof(prime));

    u32 sortedPrimeCounts0_ [64];
    u32 sortedPrimeCounts1_ [64];
    u32* sortedPrimeCountsSrc = sortedPrimeCounts0_;
    u32* sortedPrimeCountsDest = sortedPrimeCounts1_;

    // Subdivides prime counts array between indices
    for (u32 i = 0; i < 64; i++)
        sortedPrimeCounts0_[i] = sortedPrimeSize * i;

    u32 smallFactorCap = 0;
    u32 mediumFactorCap = smallFactorCount;

    // Iterates primes in sections to lower memory usage
    for (u64 i = byteStart; i <= byteEnd; i += maxSectionSize) {
        // Unmarks all numbers in the section
        memset(isFoundPrime, 0xFF, maxSectionSize);

        // Unmasks primes before the start
        if (i == byteStart) isFoundPrime[0] &= 0xFF << startInvalidBits[searchStart % 30 / 2];

        // Limits end of the section by its size and the search end
        u64 sectionSize = maxSectionSize - 1;
        u64 smallSectionSize = l1CacheSize - 1;
        if (sectionSize > byteEnd - i) {
            sectionSize = byteEnd - i;
            if (smallSectionSize > sectionSize) smallSectionSize = sectionSize;
        }

        // Caps small prime loop if the next multiple of a prime exceeds the end of the section
        for (u32 j = smallFactorCap;; j++) {
            // Exception is made if small prime distance exceeds small section size
            if (primes[j].factor * 3 + 3 >= sectionSize || j >= smallFactorCount) {
                smallFactorCap = smallFactorCount;
                break;
            }
            if (primes[j].multiple - i > sectionSize) {
                smallFactorCap = j;
                break;
            }
        }

        sieveSmallPrimes(primes, smallFactorCap, isFoundPrime, i, l1CacheSize, sectionSize);

        // Caps medium prime loop if the next multiple of a prime exceeds the end of the section
        // Medium primes are sorted by wheel to reduce branch misprediction
        for (u32 j = mediumFactorCap;; j++) {
            // Exception is made if medium prime distance exceed section size
            if (j == mediumFactorCount || primes[j].factor * 3 + 3 >= sectionSize) {
                for (u32 k = j; k < mediumFactorCount; k++) {
                    u32 wheel = primes[k].wheel;
                    sortedPrimesSrc[sortedPrimeCountsSrc[wheel]++] = primes[k];
                }
                mediumFactorCap = mediumFactorCount;
                break;
            }
            if (primes[j].multiple - i > sectionSize) {
                mediumFactorCap = j;
                break;
            }
            u32 wheel = primes[j].wheel;
            sortedPrimesSrc[sortedPrimeCountsSrc[wheel]++] = primes[j];
        }

        // Subdivides prime counts array between wheel indices
        for (u32 j = 0; j < 64; j++)
            sortedPrimeCountsDest[j] = sortedPrimeSize * j;

        sieveMediumPrimes(sortedPrimesSrc, sortedPrimeCountsSrc, sortedPrimesDest, sortedPrimeCountsDest, sortedPrimeSize, isFoundPrime, i, sectionSize);

        // Swaps pointers between medium prime arrays to avoid copying
        void* sortedSwap = sortedPrimesSrc;
        sortedPrimesSrc = sortedPrimesDest;
        sortedPrimesDest = sortedSwap;

        sortedSwap = sortedPrimeCountsSrc;
        sortedPrimeCountsSrc = sortedPrimeCountsDest;
        sortedPrimeCountsDest = sortedSwap;

        // Todo: optimize this for search ends >= 10^12
        // Large prime factors use a branchless cache friendly bucket algorithm
        /*
        for (u32 j = mediumFactorCount; j < primeFactorCount; j++) {
            u64 subPrimeFactor = primes[j].factor;
            u64 subPrimeMultiple = primes[j].multiple - i;
            u32 wheel = primes[j].wheel;
            
            while (subPrimeMultiple <= sectionSize) {
                isFoundPrime[subPrimeMultiple] &= unprimeBit[wheel];
                subPrimeMultiple += subPrimeFactor * multipleDist[wheel & 7] + multipleCorrection[wheel];
                wheel = nextWheel[wheel];
            }

            primes[j].multiple = subPrimeMultiple + i;
            primes[j].wheel = wheel;
        }
        */

        // Adds unmarked numbers to the prime count
        for (u32 j = 0; (u64) j <= sectionSize / 8; j++)
            foundPrimeCount += POPCOUNT_64(((u64*) isFoundPrime)[j]);
    }
   
    // Some bits in the first byte can be valid primes
    u32 firstInvalidPair = (u32) ((searchEnd - byteStart * 30 - 1) % (maxSectionSize * 30) / 2);
    if (firstInvalidPair % 120 > 2) {
        u64 invalidBitMask = 0xFFFFFFFFFFFFFFFF << (firstInvalidPair / 15 % 8 * 8 + endInvalidBits[firstInvalidPair % 15]);
        ((u64*) isFoundPrime)[firstInvalidPair / 120] &= invalidBitMask;

        // Uncounts primes past the search cap in the last section
        foundPrimeCount -= POPCOUNT_64(((u64*) isFoundPrime)[firstInvalidPair / 120]);
    }

    // Corrects error of not counting 2, 3, and 5 as prime
    if (searchStart < 3) foundPrimeCount += 3;
    else if (searchStart < 4) foundPrimeCount += 2;
    else if (searchStart < 6) foundPrimeCount += 1;

    // Frees remaining memory allocations
    free(primes);
    free(sortedPrimesSrc);
    free(sortedPrimesDest);
    free(isFoundPrime);
    return foundPrimeCount;
}

static void sieveSmallPrimes(prime* primes, u32 smallFactorCap, u8* isFoundPrime, u64 sectionStart, u32 l1CacheSize, u64 sectionSize) {
    for (u64 j = l1CacheSize;; j += l1CacheSize) {
        u64 lastSmallIndex = j > sectionSize ? sectionSize : j;

        // Marks non primes up to the end of the small section
        for (u32 k = 0; k < smallFactorCap; k++) {
            u64 subPrimeFactor = primes[k].factor;
            u64 subPrimeMultiple = primes[k].multiple - sectionStart;

            switch (primes[k].wheel) {
                sieveSmallPrimeFactor(7)
                sieveSmallPrimeFactor(11)
                sieveSmallPrimeFactor(13)
                sieveSmallPrimeFactor(17)
                sieveSmallPrimeFactor(19)
                sieveSmallPrimeFactor(23)
                sieveSmallPrimeFactor(29)
                sieveSmallPrimeFactor(1)
                default: UNREACHABLE
            }

            nextSmallPrimeFactor:
            primes[k].multiple = subPrimeMultiple + sectionStart;
        }

        if (j >= sectionSize) break;
    }
}

static void sieveMediumPrimes(prime* sortedPrimesSrc, u32* sortedPrimeCountsSrc, prime* sortedPrimesDest, u32* sortedPrimeCountsDest, 
    u32 sortedPrimeSize, u8* isFoundPrime, u64 sectionStart, u64 sectionSize) {
    for (u32 j = 0; j < 64; j++)
        for (u32 k = j * sortedPrimeSize; k < sortedPrimeCountsSrc[j]; k++) {
            u64 subPrimeFactor = sortedPrimesSrc[k].factor;
            u64 subPrimeMultiple = sortedPrimesSrc[k].multiple - sectionStart;

            // Marks non primes up to the end of the section
            u32 wheel;
            switch (j) {
                sieveMediumPrimeFactor(7)
                sieveMediumPrimeFactor(11)
                sieveMediumPrimeFactor(13)
                sieveMediumPrimeFactor(17)
                sieveMediumPrimeFactor(19)
                sieveMediumPrimeFactor(23)
                sieveMediumPrimeFactor(29)
                sieveMediumPrimeFactor(1)
                default: UNREACHABLE
            }

            nextLargePrimeFactor:
            sortedPrimesDest[sortedPrimeCountsDest[wheel]++] = (prime) {0, (u32) subPrimeFactor, subPrimeMultiple + sectionStart};
        }
}
