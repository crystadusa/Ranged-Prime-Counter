cmake_minimum_required(VERSION 3.21)
project("Ranged prime counter" LANGUAGES C)

# Sets c compiler flags 
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # Sets c compiler flags for gcc
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(WARN_FLAGS "-Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable")
        set(COMMON_FLAGS "${WARN_FLAGS} -fverbose-asm -masm=intel")
        set(SMALL_OPTIMIZE_FLAGS "${COMMON_FLAGS} -Os -march=nehalem -mtune=native -fallow-store-data-races -fipa-pta -fira-hoist-pressure \
            -flto -fmerge-all-constants -fmodulo-sched -fmodulo-sched-allow-regmoves -fomit-frame-pointer -ftree-vectorize -fvect-cost-model=dynamic \
            -fno-ident -fno-math-errno -fno-trapping-math -fno-signed-zeros -fassociative-math -ffinite-math-only -Wl,--gc-sections")
        set(OPTIMIZE_FLAGS "${SMALL_OPTIMIZE_FLAGS} -O2")

        set(PROFILE_FLAGS "-fno-align-functions -fno-align-loops -fno-align-jumps -fno-align-labels")
        set(C_FLAGS_GENPROFILE "-fprofile-generate ${PROFILE_FLAGS}")
        set(C_FLAGS_USEPROFILE "-fprofile-use -fprofile-correction ${PROFILE_FLAGS}")

    # Sets c compiler flags for clang,
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(WARN_FLAGS "-Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable")
        set(COMMON_FLAGS "${WARN_FLAGS} -fuse-ld=lld -fverbose-asm -masm=intel")
        set(SMALL_OPTIMIZE_FLAGS "${COMMON_FLAGS} -Os -march=nehalem -mtune=native -flto -fmerge-all-constants -fomit-frame-pointer -ftree-vectorize \
            -fno-asynchronous-unwind-tables -fno-math-errno -fno-trapping-math -fno-signed-zeros -fassociative-math -ffinite-math-only")
        set(OPTIMIZE_FLAGS "${SMALL_OPTIMIZE_FLAGS} -O2")

        set(C_FLAGS_GENPROFILE "-fprofile-generate")
        set(C_FLAGS_USEPROFILE "-fprofile-use=Prime-BuildPrime.profdata")
    endif()

    # Sets all build target c flags for the gcc and clang
    set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS_ASM "${OPTIMIZE_FLAGS} -fno-lto -s")
    set(CMAKE_C_FLAGS_DEBUG "${COMMON_FLAGS} -g")
    set(CMAKE_C_FLAGS_RELEASE "${OPTIMIZE_FLAGS} -s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${SMALL_OPTIMIZE_FLAGS} -s")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${OPTIMIZE_FLAGS} -g")
    set(CMAKE_C_FLAGS_GENPROFILE "${OPTIMIZE_FLAGS}")
    set(CMAKE_C_FLAGS_USEPROFILE "${OPTIMIZE_FLAGS}")
endif ()

# Sets c compiler flags for msvc
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(COMMON_FLAGS "/nologo /D_CRT_SECURE_NO_WARNINGS /W4 /wd4245 /permissive-")
    set(SMALL_OPTIMIZE_FLAGS "${COMMON_FLAGS} /O2 /Os")
    set(OPTIMIZE_FLAGS "${SMALL_OPTIMIZE_FLAGS} /Ot")

    set(C_FLAGS_GENPROFILE "/GL")
    set(LINK_FLAGS_GENPROFILE "/GENPROFILE /LTCG")
    set(C_FLAGS_USEPROFILE "/GL")
    set(LINK_FLAGS_GENPROFILE "/GENPROFILE /LTCG")

    set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
    set(CMAKE_C_FLAGS_ASM "${OPTIMIZE_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "${COMMON_FLAGS} /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${OPTIMIZE_FLAGS}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${SMALL_OPTIMIZE_FLAGS}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${OPTIMIZE_FLAGS} /Zi")
endif()

# Sets relative file paths from binary to source directory
file(RELATIVE_PATH BASE ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})
file(RELATIVE_PATH SRC ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)

# Builds and runs MakePrime to build Prime
add_executable(MakePrime ${SRC}/MakePrime.c)

add_custom_command(
    OUTPUT ${SRC}/BuildPrime.c
    DEPENDS MakePrime
    COMMAND MakePrime ${SRC}/Prime.c ${SRC}/BuildPrime.c
)

# PGO stands for profile guided optimization
# Builds Prime with dependency on profiling for PGO
if (CMAKE_BUILD_TYPE STREQUAL "USEPROFILE")
    add_executable(Prime Profile ${SRC}/BuildPrime.c)
    set_target_properties(Prime PROPERTIES COMPILE_FLAGS "${C_FLAGS_USEPROFILE}")
    set_target_properties(Prime PROPERTIES LINK_FLAGS "${LINK_FLAGS_USEPROFILE}")

    # Removes dump files post build
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_custom_command(
            TARGET Prime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove Prime-BuildPrime.gcda
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        add_custom_command(
            TARGET Prime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove Prime-BuildPrime.profdata *.profraw
        )
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_custom_command(
            TARGET Prime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove release/prime.pgd 
                release/prime!1.pgc release/prime!2.pgc release/prime!3.pgc release/prime!4.pgc release/prime!5.pgc
                release/prime!6.pgc release/prime!7.pgc release/prime!8.pgc release/prime!9.pgc release/prime!10.pgc
        )
    endif()

else ()
    add_executable(Prime ${SRC}/BuildPrime.c)
endif ()

# Sets compiler flags and libraries for PGO
if (CMAKE_BUILD_TYPE STREQUAL "GENPROFILE")
    set_target_properties(Prime PROPERTIES COMPILE_FLAGS "${C_FLAGS_GENPROFILE}")
    set_target_properties(Prime PROPERTIES LINK_FLAGS "${LINK_FLAGS_GENPROFILE}")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(Prime PRIVATE "gcov")
    elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_link_libraries(Prime PRIVATE "libclang_rt.profile-x86_64.a")
    endif()
endif()

# Copies the dll for pthreads to the executable for msvc
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_include_directories(Prime PRIVATE "pthread/include")
    target_link_libraries(Prime PRIVATE "${BASE}/pthread/lib/x64/pthreadVC2")
    add_custom_command(
        TARGET Prime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BASE}/pthread/dll/x64/pthreadVC2.dll $<TARGET_FILE_DIR:Prime>
    )
endif ()

# Profiles the executable with a build target for gcc and msvc
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_custom_command(
        OUTPUT Profile
        COMMAND Prime 00000000000 10000000000
        COMMAND Prime 10000000000 20000000000
        COMMAND Prime 20000000000 30000000000
        COMMAND Prime 30000000000 40000000000
        COMMAND Prime 40000000000 50000000000
        COMMAND Prime 50000000000 60000000000
        COMMAND Prime 60000000000 70000000000
        COMMAND Prime 70000000000 80000000000
        COMMAND Prime 80000000000 90000000000
        COMMAND Prime 90000000000 100000000000
    )
endif()

# Profiles the executable with a build target for clang
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_custom_command(
        OUTPUT Profile
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime0.profraw Prime 00000000000 10000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime1.profraw Prime 10000000000 20000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime2.profraw Prime 20000000000 30000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime3.profraw Prime 30000000000 40000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime4.profraw Prime 40000000000 50000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime5.profraw Prime 50000000000 60000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime6.profraw Prime 60000000000 70000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime7.profraw Prime 70000000000 80000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime8.profraw Prime 80000000000 90000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime9.profraw Prime 90000000000 100000000000
        COMMAND llvm-profdata merge *.profraw -o=Prime-BuildPrime.profdata
    )
endif()