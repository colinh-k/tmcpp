add_library(tmcpp.tmcpp INTERFACE)
add_library(tmcpp::tmcpp ALIAS tmcpp.tmcpp)
target_sources(tmcpp.tmcpp
    PUBLIC
        FILE_SET
        HEADERS
        FILES
            tmcpp/tmcpp.hpp
            tmcpp/concepts.hpp
            tmcpp/consteval_string.hpp
            tmcpp/algorithm.hpp
            tmcpp/list.hpp
            tmcpp/multilambda.hpp
        BASE_DIRS
)
# NOTE: this enable headers to be verified with the compiler, even if they are not directly included in any part of the project
set_target_properties(tmcpp.tmcpp
    PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS
            ON
)
