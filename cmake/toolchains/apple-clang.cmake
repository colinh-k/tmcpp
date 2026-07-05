# sets the compiler environment to build the tests on a host machine. since this library is intended to be used on embedded targets, it should also be compiled using a cross compiler with appropriate flags, so we know the library can successfully cross compile with the strict target flags, eg -fno-exceptions, etc
# however, when testing, we can enable sanitizers for extra correctness checks

set(CMAKE_C_COMPILER cc)
set(CMAKE_CXX_COMPILER c++)

# configure sanitizers for this toolchain
# TODO: wrap each of these sanitizer names around a check to see if a corresponding option is set, ie ENABLE_SANITIZER_ADDRESS for 'address' sanitizer. this lets the user select which sanitizers to add at configure time. (note: i wrote it in this verbose way to make it easier to add those checks later, instead of just writing the santizer flags with this short list of known flags)
set(TOOLCHAIN_SANITIZERS "")
list(APPEND TOOLCHAIN_SANITIZERS "address")
list(APPEND TOOLCHAIN_SANITIZERS "undefined")
list(APPEND TOOLCHAIN_SANITIZERS "pointer-compare")
list(APPEND TOOLCHAIN_SANITIZERS "pointer-subtract")
list(JOIN TOOLCHAIN_SANITIZERS "," TOOLCHAIN_SANITIZERS_LIST)
if(TOOLCHAIN_SANITIZERS_LIST)
    set(TOOLCHAIN_SANITIZER_FLAGS -fsanitize=${TOOLCHAIN_SANITIZERS_LIST})
endif()

add_compile_options(
    $<$<CONFIG:debug>:-O0>
    $<$<CONFIG:debug>:-g3>

    $<$<CONFIG:release>:-O3>
    $<$<CONFIG:release>:-g0>

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
    -Wno-unused-parameter
    -Wno-error=unused-parameter
    # disable abi warnings as we always compile the whole programm and do not link precompiled libraries
    -Wno-psabi
    # warn the user if a class with virtual functions has a non-virtual destructor.
    "$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>"
    "$<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>"
    # warn if you overload (not override) a virtual function
    "$<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>"

    # clang-specific warnings
    -Wno-unused-private-field
    -Wno-error=unused-private-field

    # NOTE: it seems we cannot use option() or cache variables in a toolchain file. so, we just have to assume the user will pass this var in
    # TODO: a potential solution: define the option() AFTER the toolchain file gets processed, ie after project() call in the main cmake list
    $<$<BOOL:${PROJECT_ENABLE_WARNINGS_AS_ERRORS}>:-Werror>

    ${TOOLCHAIN_SANITIZER_FLAGS}
)
add_link_options(
    ${TOOLCHAIN_SANITIZER_FLAGS}
)
