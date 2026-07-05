#include "tmcpp/constexpr_string.hpp"

#include <gtest/gtest.h>

using namespace tmcpp::literals;

TEST(constexpr_string, udl)
{
    constexpr auto str1 = "hewow"_L;
    constexpr auto str2 = "hewow"_L;
    constexpr auto str3 = "hewow2"_L;

    ASSERT_TRUE(str1 == str2);
    ASSERT_FALSE(str1 == str3);
}

template <tmcpp::ConstexprString A, tmcpp::ConstexprString B>
constexpr auto
foo()
{
    return A == B;
}

TEST(constexpr_string, can_be_used_as_nttp)
{
    constexpr auto str1 = "label1"_L;
    constexpr auto str2 = "label2"_L;

    static_assert(not foo<str1, str2>());
}
