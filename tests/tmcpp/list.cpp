#include "tmcpp/list.hpp"

#include <gtest/gtest.h>

#include <tuple>

TEST(ListLike, accepts_templates_with_arbitrary_type_parameters)
{
    // test empty list
    static_assert(tmcpp::ListLike<tmcpp::list<>>);
    static_assert(tmcpp::ListLike<tmcpp::list<int>>);
    static_assert(
        tmcpp::ListLike<tmcpp::list<int, bool, double, float, double>>);

    // test non-tmcpp list types
    static_assert(tmcpp::ListLike<std::tuple<>>);
    static_assert(tmcpp::ListLike<std::tuple<float>>);
    static_assert(tmcpp::ListLike<std::tuple<bool, double, unsigned, long>>);
}

TEST(is_empty_v, works_on_list_and_tuple)
{
    static_assert(tmcpp::is_empty_v<tmcpp::list<>>);
    static_assert(not tmcpp::is_empty_v<tmcpp::list<int>>);
    static_assert(not tmcpp::is_empty_v<tmcpp::list<bool, double, float>>);

    static_assert(tmcpp::is_empty_v<std::tuple<>>);
    static_assert(not tmcpp::is_empty_v<std::tuple<int>>);
    static_assert(not tmcpp::is_empty_v<std::tuple<bool, double, float>>);
}

TEST(size_v, works_on_list_and_tuple)
{
    static_assert(tmcpp::size_v<tmcpp::list<>> == 0);
    static_assert(tmcpp::size_v<tmcpp::list<int>> == 1);
    static_assert(tmcpp::size_v<tmcpp::list<bool, double, float>> == 3);

    static_assert(tmcpp::size_v<std::tuple<>> == 0);
    static_assert(tmcpp::size_v<std::tuple<int>> == 1);
    static_assert(tmcpp::size_v<std::tuple<bool, double, float>> == 3);
}

TEST(at, works_on_list_and_tuple)
{
    static_assert(std::is_same_v<tmcpp::at<0, tmcpp::list<int>>, int>);
    using list = tmcpp::list<bool, double, float>;
    static_assert(std::is_same_v<tmcpp::at<0, list>, bool>);
    static_assert(std::is_same_v<tmcpp::at<1, list>, double>);
    static_assert(std::is_same_v<tmcpp::at<2, list>, float>);

    static_assert(std::is_same_v<tmcpp::at<0, std::tuple<int>>, int>);
    using tuple = std::tuple<bool, double, float>;
    static_assert(std::is_same_v<tmcpp::at<0, tuple>, bool>);
    static_assert(std::is_same_v<tmcpp::at<1, tuple>, double>);
    static_assert(std::is_same_v<tmcpp::at<2, tuple>, float>);
}

template <typename List>
consteval auto
get_first()
{
    static_assert(not tmcpp::is_empty_v<List>);

    using first = tmcpp::at<0, List>;
    return first{};
}

TEST(at, within_template_function)
{
    using L = tmcpp::list<bool, int, double, float>;

    using expected = bool;
    using actual = decltype(get_first<L>());

    static_assert(std::is_same_v<actual, expected>);
}

template <typename List>
using get_first_alias = decltype([](){
    static_assert(not tmcpp::is_empty_v<List>);

    using first = tmcpp::at<0, List>;
    return first{};
}());

TEST(at, within_template_type_alias)
{
    using L = tmcpp::list<bool, int, double, float>;

    using expected = bool;
    using actual = get_first_alias<L>;

    static_assert(std::is_same_v<actual, expected>);
}
