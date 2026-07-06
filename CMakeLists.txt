cmake_minimum_required(VERSION 3.31)
project(tmcpp
    LANGUAGES
        CXX
)

option(TMCPP_ENABLE_TESTING "enable building and running tests" FALSE)
option(TMCPP_ENABLE_WARNINGS_AS_ERRORS "interpret compiler warnings as errors" TRUE)

# TODO: notice that we allow the user to specify the c++ standard version. as such, we need to find some way to assert that the c++ standard features the library uses are present in the current build configuration.

add_subdirectory(include)

if(TMCPP_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

