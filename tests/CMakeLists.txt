# these configuration steps are copied from the google test documentation, using the latest git hash at the time of writing
include(FetchContent)
FetchContent_Declare(googletest
    URL
        https://github.com/google/googletest/archive/d72f9c8aea6817cdf1ca0ac10887f328de7f3da2.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(all-tests)
target_sources(all-tests
    PRIVATE
        tmcpp/consteval_string.cpp
        tmcpp/tmcpp.cpp
)
target_link_libraries(all-tests
    tmcpp::tmcpp
    GTest::gtest_main
)

# using these options, we do not have to write a main() fn in the test code.
# when using auto-detected tests (ie with no custom main()), we have to force gtest to use colorful output with a flag
include(GoogleTest)
gtest_discover_tests(all-tests
    EXTRA_ARGS
        --gtest_color=yes
)
