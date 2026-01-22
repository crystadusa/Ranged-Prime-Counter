cmake_minimum_required(VERSION 3.21)
project("Ranged prime counter" LANGUAGES C)

message("CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message("CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")

# Sets c compiler flags 
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # Sets c compiler flags for gcc
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(WARN_FLAGS "-Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable")
        set(COMMON_FLAGS "${WARN_FLAGS} -fverbose-asm -masm=intel")
        set(SMALL_OPTIMIZE_FLAGS "${COMMON_FLAGS} -Os -march=nehalem -mtune=native -flto -fno-math-errno -fno-trapping-math -Wl,--gc-sections")
        set(OPTIMIZE_FLAGS "${SMALL_OPTIMIZE_FLAGS} -O2")

        set(PROFILE_FLAGS "-fno-align-functions -fno-align-loops -fno-align-jumps -fno-align-labels")
        set(C_FLAGS_GENPROFILE "-fprofile-generate ${PROFILE_FLAGS}")
        set(C_FLAGS_USEPROFILE "-fprofile-use -fprofile-correction ${PROFILE_FLAGS}")

    # Sets c compiler flags for clang,
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        set(WARN_FLAGS "-Wall -Wextra -pedantic -Wconversion -Wdouble-promotion -Wshadow -Wno-cast-function-type -Wno-sign-conversion -Wno-unused-variable")
        set(COMMON_FLAGS "${WARN_FLAGS} -fuse-ld=lld -fverbose-asm -masm=intel")
        set(SMALL_OPTIMIZE_FLAGS "${COMMON_FLAGS} -Os -march=nehalem -mtune=native -flto -fno-asynchronous-unwind-tables \
            -fno-math-errno -fno-trapping-math")
        set(OPTIMIZE_FLAGS "${SMALL_OPTIMIZE_FLAGS} -O2")

        set(C_FLAGS_GENPROFILE "-fprofile-generate")
        set(C_FLAGS_USEPROFILE "-fprofile-use=Prime-BuildPrime.profdata")
    endif()

    # Sets output directory for release and debug builds
    if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
    else ()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
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
    set(COMMON_FLAGS "/MD /nologo /D_CRT_SECURE_NO_WARNINGS /W4 /wd4245 /permissive-")
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

    add_link_options("/Manifest:No")
endif()

# Sets relative file paths from binary to source directory
file(RELATIVE_PATH BASE ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/..)
file(RELATIVE_PATH SRC ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/../src)

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

# Profiles the executable with a build target for gcc and msvc
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_custom_command(
        OUTPUT Profile
        COMMAND Prime 10000000000000 10010000000000
        COMMAND Prime 10010000000000 10020000000000
        COMMAND Prime 10020000000000 10030000000000
        COMMAND Prime 10030000000000 10040000000000
        COMMAND Prime 10040000000000 10050000000000
        COMMAND Prime 10050000000000 10060000000000
        COMMAND Prime 10060000000000 10070000000000
        COMMAND Prime 10070000000000 10080000000000
        COMMAND Prime 10080000000000 10090000000000
        COMMAND Prime 10090000000000 10100000000000
    )
endif()

# Profiles the executable with a build target for clang
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_custom_command(
        OUTPUT Profile
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime0.profraw Prime 10000000000000 10010000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime1.profraw Prime 10010000000000 10020000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime2.profraw Prime 10020000000000 10030000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime3.profraw Prime 10030000000000 10040000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime4.profraw Prime 10040000000000 10050000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime5.profraw Prime 10050000000000 10060000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime6.profraw Prime 10060000000000 10070000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime7.profraw Prime 10070000000000 10080000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime8.profraw Prime 10080000000000 10090000000000
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=Prime-BuildPrime9.profraw Prime 10090000000000 10100000000000
        COMMAND llvm-profdata merge *.profraw -o=Prime-BuildPrime.profdata
    )
endif()