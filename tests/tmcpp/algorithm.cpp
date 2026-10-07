#include "tmcpp/algorithm.hpp"
#include "tmcpp/tmcpp.hpp"

#include <gtest/gtest.h>

#include <type_traits>

TEST(rename, list_to_tuple_simple)
{
    using list = tmcpp::list<bool, float, int>;

    using expected = std::tuple<bool, float, int>;
    using actual = tmcpp::rename<list, std::tuple>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(rename, tuple_to_list_simple)
{
    using tuple = std::tuple<bool, float, int>;

    using expected = tmcpp::list<bool, float, int>;
    using actual = tmcpp::rename<tuple, tmcpp::list>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(rename, list_to_tuple_empty)
{
    using list = tmcpp::list<>;

    using expected = std::tuple<>;
    using actual = tmcpp::rename<list, std::tuple>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(concatenate, simple)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<float, long>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected
        = tmcpp::list<int, bool, double, float, long, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(concatenate, with_empty_list)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected = tmcpp::list<int, bool, double, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(concatenate, with_no_lists)
{
    using Actual = tmcpp::concatenate<>;
    using Expected = tmcpp::list<>;

    static_assert(std::is_same_v<Actual, Expected>);
}

TEST(front_or, non_empty)
{
    using L = tmcpp::list<double, float, int>;

    using expected = double;
    using actual = tmcpp::front_or<L, unsigned>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(front_or, empty)
{
    using L = tmcpp::list<>;

    using expected = unsigned;
    using actual = tmcpp::front_or<L, unsigned>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(front_or, empty_with_void_default)
{
    using L = tmcpp::list<>;

    using expected = void;
    using actual = tmcpp::front_or<L, void>;

    static_assert(std::is_same_v<actual, expected>);
}

template <typename A, typename B> using Equivalent = std::is_same<A, B>;

template <typename T> struct type_equal_to
{
    template <typename U> using invoke = std::is_same<T, U>;
};

#if 0
TEST(filter, empty)
{
    using L = tmcpp::list<>;

    using expected = tmcpp::list<>;
    using actual = tmcpp::filter<type_equal_to<int>, L>;

    static_assert(std::is_same_v<actual, expected>);
}
#endif

TEST(filter, lambda_simple)
{
    using list = tmcpp::list<bool, int, double, bool, float, bool>;

    using expected = tmcpp::list<bool, bool, bool>;
    // finds all types in the list that are bool
    using actual = tmcpp::filter<[]<typename T>
                                 { return std::is_same_v<T, bool>; }, list>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(find_if, simple_lambda)
{
    using list = tmcpp::list<int, bool, double, bool, float>;

    using expected = bool;
    using actual = tmcpp::find_if<[]<typename T>
                                  { return std::is_same_v<T, bool>; }, list>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(find_if, none)
{
    using list = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::not_found;
    using actual
        = tmcpp::find_if<[]<typename T>
                         { return std::is_same_v<T, unsigned>; }, list>;

    static_assert(std::is_same_v<actual, expected>);
}

// TODO: add more tests for *_index algorithms
TEST(filter_index, simple_lambda)
{
    using list = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::list<std::integral_constant<std::size_t, 1>,
                                 std::integral_constant<std::size_t, 3>>;
    using actual
        = tmcpp::filter_index<[]<typename T>
                              { return std::is_same_v<T, bool>; }, list>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(filter_index, returns_empty_list_if_no_types_satisfy_predicate)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::list<>;
    using actual
        = tmcpp::filter_index<[]<typename T>
                              { return std::is_same_v<T, unsigned>; }, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(find_index_if, simple_lambda)
{
    using list = tmcpp::list<int, bool, double, bool, float>;

    using expected = std::integral_constant<std::size_t, 2>;
    using actual
        = tmcpp::find_index_if<[]<typename T>
                               { return std::is_same_v<T, double>; }, list>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(find_index_if, not_found)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::not_found;
    using actual
        = tmcpp::find_index_if<[]<typename T>
                               { return std::is_same_v<T, unsigned>; }, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(transform, simple_lambda)
{
    using List = tmcpp::list<int, double, float, bool, unsigned>;

    using expected = tmcpp::list<int *, double *, float *, bool *, unsigned *>;
    using actual = tmcpp::transform<[]<typename T> -> T * {}, List>;

    static_assert(std::is_same_v<actual, expected>);
}

// for now, we cant pass empty lists due to the concept constraint
#if 0
TEST(transform, empty_list)
{
    using List = tmcpp::list<>;

    using expected = tmcpp::list<>;
    using actual = tmcpp::transform<[]<typename T> -> T * {}, List>;

    static_assert(std::is_same_v<actual, expected>);
}
#endif

struct type_comparator
{
    template <typename T, typename U> using invoke = std::is_same<T, U>;
};

TEST(append_if_unique, appends_if_type_not_in_list_already)
{
    using L = tmcpp::list<int>;

    using expected = tmcpp::list<int, bool>;
    using actual = tmcpp::append_if_unique<type_comparator, L, bool>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(append_if_unique, does_not_append_if_type_in_list_already)
{
    using L = tmcpp::list<int>;

    using expected = tmcpp::list<int>;
    using actual = tmcpp::append_if_unique<type_comparator, L, int>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(append_if_unique, on_empty_list)
{
    using L = tmcpp::list<>;

    using expected = tmcpp::list<int>;
    using actual = tmcpp::append_if_unique<type_comparator, L, int>;

    static_assert(std::is_same_v<actual, expected>);
}

// TODO: for now, the remove_duplicates implemntation retains the LAST instance
// of a type, so we check for that here. if we change the implmentation later
// to keep the FIRST instance of a type, we will need to refactor the expected
// lists
TEST(remove_duplicates, simple)
{
    using L = tmcpp::list<int, double, int, bool, bool>;

    using expected = tmcpp::list<int, double, bool>;
    using actual = tmcpp::remove_duplicates<L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(remove_duplicates, empty)
{
    using L = tmcpp::list<>;

    using expected = tmcpp::list<>;
    using actual = tmcpp::remove_duplicates<L>;

    static_assert(std::is_same_v<actual, expected>);
}
