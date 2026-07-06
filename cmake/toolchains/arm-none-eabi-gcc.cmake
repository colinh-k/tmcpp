set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
# do not attempt to perform a compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# locate the cross compiler directory, eg /path/to/toolchain/bin/arm-none-eabi-gcc
# NOTE: this discovery process assumes arm-none-eabi-gcc is located in /path/to/toolchain/bin/arm-none-eabi-gcc, and /path/to/toolchain/ directory has a subdirectory called arm-none-eabi, which we use below as the sysroot for system headers
find_program(TOOLCHAIN_GCC
    NAMES
        arm-none-eabi-gcc
    REQUIRED
)
# extract /path/to/toolchain to get the base directory of the cross compiler
cmake_path(GET TOOLCHAIN_GCC PARENT_PATH TOOLCHAIN_BIN_DIR)
cmake_path(GET TOOLCHAIN_BIN_DIR PARENT_PATH TOOLCHAIN_ROOT_DIR)
set(TOOLCHAIN_PREFIX ${TOOLCHAIN_BIN_DIR}/arm-none-eabi)

# this tells cmake where to find system header files. without this, cmake would use the host's system headers, which are not used in cross compilation.
set(CMAKE_SYSROOT ${TOOLCHAIN_ROOT_DIR}/arm-none-eabi/)

set(CMAKE_C_COMPILER_ID   GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_LINKER       ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}-objcopy)
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}-size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

# MCU specific flags
set(TARGET_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
# NOTE: this should be updated if we move the linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F303XX_FLASH.ld)

add_compile_options(
    "$<$<COMPILE_LANGUAGE:C,CXX,ASM>:${TARGET_FLAGS}>"

    $<$<CONFIG:debug>:-O0>
    $<$<CONFIG:debug>:-g3>

    $<$<CONFIG:release>:-O3>
    $<$<CONFIG:release>:-g0>

    "$<$<COMPILE_LANGUAGE:C,CXX>:-fdata-sections>"
    "$<$<COMPILE_LANGUAGE:C,CXX>:-ffunction-sections>"
    "$<$<COMPILE_LANGUAGE:C,CXX>:-fanalyzer>"

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
    $<$<BOOL:${TMCPP_ENABLE_WARNINGS_AS_ERRORS}>:-Werror>

    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>"
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>"
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>"

    "$<$<COMPILE_LANGUAGE:ASM>:-x>"
    "$<$<COMPILE_LANGUAGE:ASM>:assembler-with-cpp>"
    "$<$<COMPILE_LANGUAGE:ASM>:-MMD>"
    "$<$<COMPILE_LANGUAGE:ASM>:-MP>"
)
add_link_options(
    ${TARGET_FLAGS}
    -T ${LINKER_SCRIPT}
    --specs=nano.specs
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
)
