# NOTE: due to the struggles involved with installing a new gnu compiler version and updating the default gcc/g++ commands, our project will just inherit the CC/CXX variables from the environment to find the proper compiler. please select a recent gcc version >=14
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

# these must be passed to both compiler and linker
set(TOOLCHAIN_SANITIZER_FLAGS -fsanitize=address,pointer-compare,pointer-subtract,undefined)
set(TOOLCHAIN_WARNING_FLAGS -Wall -Wextra -Wshadow -Wpedantic -Wconversion -Werror)

add_compile_options(
    $<$<CONFIG:debug>:-O0>
    $<$<CONFIG:debug>:-g3>

    $<$<CONFIG:release>:-O3>
    $<$<CONFIG:release>:-g0>

    # "$<$<COMPILE_LANGUAGE:C,CXX>:-fanalyzer>"

    -Wall
    -Wextra
    -Wshadow
    # warn for potential performance problem casts
    -Wcast-align
    -Wunused
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    # warn if float is implicit promoted to double
    -Wdouble-promotion
    # warn on security issues around functions that format output (ie printf)
    -Wformat=2
    -Wtype-limits
    # TODO: do i really want to disable warnings for unused parameter ?
    -Wno-unused-parameter
    -Wno-error=unused-parameter
    # disable abi warnings as we always compile the whole programm and do not link precompiled libraries
    -Wno-psabi
    # warn the user if a class with virtual functions has a non-virtual destructor.
    "$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>"
    "$<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>"
    # warn if you overload (not override) a virtual function
    "$<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>"
    $<$<BOOL:${PROJECT_ENABLE_WARNINGS_AS_ERRORS}>:-Werror>

    # NOTE: we can either enable "-fsanitize=thread" xor these sanitizers
    "$<$<COMPILE_LANGUAGE:C,CXX>:${TOOLCHAIN_SANITIZER_FLAGS}>"
)
add_link_options(
    "$<$<COMPILE_LANGUAGE:C,CXX>:${TOOLCHAIN_SANITIZER_FLAGS}>"
)
