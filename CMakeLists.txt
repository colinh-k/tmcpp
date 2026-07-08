cmake_minimum_required(VERSION 3.31)
project(tmcpp
    LANGUAGES
        CXX
)

option(TMCPP_ENABLE_TESTING "enable building and running tests" FALSE)
option(TMCPP_ENABLE_WARNINGS_AS_ERRORS "interpret compiler warnings as errors" TRUE)

# TODO: we need to make sure we have at least gcc-14 or clang-21, since older versions have bugs that dont compile this project. where is the best place to put this check ? it should probably be here since if we put the check in the toolchain file, then the user could simply use a different toolchain file in their project, bypassing the check. but putting it here is kinda messy since we have to check which compiler is being used
# TODO: i feel like there should be a better way to assert the compiler is at least a certain version...
if ((CMAKE_CXX_COMPILER_ID STREQ "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0"))
    message(FATAL_ERROR "gnu compiler requires at least version 14")
# TODO: earlier versions of clang might work, but i only tested 21 and that seems to work
elseif((CMAKE_CXX_COMPILER_ID STREQ "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "21.0"))
    message(FATAL_ERROR "clang compiler requires at least version 21")
endif()

# TODO: notice that we allow the user to specify the c++ standard version. as such, we need to find some way to assert that the c++ standard features the library uses are present in the current build configuration.

add_subdirectory(include)

if(TMCPP_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

