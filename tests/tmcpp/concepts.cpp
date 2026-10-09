#include "tmcpp/concepts.hpp"

#include <concepts>
#include <gtest/gtest.h>

// TODO: we need tests to check that common mistakes dont compile, eg checking
// that a predicate accepts every expected type in a list

// just a simple test
TEST(TypeFunctionFor, lambda_accepts_single_type_argument)
{
    // inferred from return type
    static_assert(tmcpp::TypeFunctionFor<[]<typename T> -> T {}, int>);
    // inferred from body
    static_assert(tmcpp::TypeFunctionFor<[]<typename T> { return T{}; }, int>);
}

TEST(TypeFunctionFor, lambda_accepts_multiple_type_arguments)
{
    // returns a list of types in reverse order just for fun
    static_assert(
        tmcpp::TypeFunctionFor<
            []<typename T, typename U, typename V> -> tmcpp::list<V, U, T> {},
            int, double, bool>);
    static_assert(tmcpp::TypeFunctionFor<[]<typename T, typename U, typename V>
                                         { return tmcpp::list<V, U, T>{}; },
                                         int, double, bool>);
}

struct NonDefaultCtorType
{
    NonDefaultCtorType() = delete;
};

TEST(TypeFunctionFor, non_default_ctor_type)
{
    static_assert(
        tmcpp::TypeFunctionFor<[]<typename T> -> T {}, NonDefaultCtorType>);
    // NOTE: if we use the return-type-inferred-from-body technique, then we
    // must constrain the lambda to check T is default initializable. if we
    // dont, then the compiler yields a hard error during concept checking,
    // which defeats the purpose of concepts
    static_assert(
        not tmcpp::TypeFunctionFor<[]<std::default_initializable T>
                                   { return T{}; }, NonDefaultCtorType>);
}

TEST(TypeFunctionFor, rejects_lambda_that_takes_non_template_parameter)
{
    static_assert(
        not tmcpp::TypeFunctionFor<[]<typename T>(T x) -> T {}, double>);
    static_assert(
        not tmcpp::TypeFunctionFor<[]<typename T>(T x) { return x; }, double>);
}

TEST(TypeFunctionFor, rejects_if_lambda_not_compatible_with_type)
{
    // notice we can constrain the type argument of the lambda
    static_assert(
        not tmcpp::TypeFunctionFor<[]<std::floating_point T> -> T {}, int>);
    // example with accepting with constrained type argument
    static_assert(
        tmcpp::TypeFunctionFor<[]<std::floating_point T> -> T {}, double>);
}

// TODO: add tests for all concepts
