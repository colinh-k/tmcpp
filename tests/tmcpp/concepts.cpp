#include "tmcpp/concepts.hpp"
#include "tmcpp/algorithm.hpp"
#include "tmcpp/tmcpp.hpp"

#include <gtest/gtest.h>

// example predicates

struct type_equal_to_int
{
    template <typename U> using invoke = std::is_same<int, U>;
};

template <typename T> struct type_equal_to
{
    template <typename U> using invoke = std::is_same<T, U>;
};

// TODO: we need tests to check that common mistakes dont compile, eg checking
// that a predicate accepts every expected type in a list

TEST(concepts, predicate_metafunction_for_simple)
{
    // just check that type_equal_to_int is a predicate for some arbitrary
    // builtin types
    static_assert(
        tmcpp::concepts::predicate_metafunction_for<type_equal_to_int, int,
                                                    double, bool, float>);
}

TEST(concepts, predicate_metafunction_for_with_template_class)
{
    // check that we can generate a predicate from a template class for some
    // arbitrary builtin types.
    // useful for when you want a predicate to be templated on some template
    // argument in a local scope (but its illegal to declare a template struct
    // in local scope, so the solution is to declare a template class predicate
    // and pass the auxiliary template argument at the call site)
    static_assert(
        tmcpp::concepts::predicate_metafunction_for<type_equal_to<double>, int,
                                                    double, bool, float>);
}

TEST(concepts, predicate_metafunction_for_list_simple)
{
    // just check that type_equal_to_int is a predicate for some arbitrary
    // builtin types
    using L = tmcpp::list<int, double, bool, float>;
    static_assert(
        tmcpp::concepts::predicate_metafunction_for_list<type_equal_to_int, L,
                                                         tmcpp::list>);
}

// compile this example under clang to see a mysterious error involving 'const
// auto'; you must NOT use 'auto' for template members of template classes (ie
// use the explicit type instead)
#if 0
template <typename U> struct foo
{
    template <typename T> static constexpr auto value = true;
};

TEST(concepts, predicate_metafunction_for)
{
    using X = decltype(foo<void>::template value<void>);
    X x = true;

    static_assert(std::is_same_v<X, const bool>);
    static_assert(std::is_convertible_v<X, bool>);

    // using P1 = type_equal_to<int>;

    // static_assert(
    //     std::is_convertible_v<decltype(P1::template value<int>), bool>);

    // static_assert(tmcpp::concepts::predicate_metafunction_for<P1, int>);
}
#endif
