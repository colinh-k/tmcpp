#include "tmcpp/consteval_string.hpp"

#include <gtest/gtest.h>

using namespace tmcpp::literals;

TEST(consteval_string, udl)
{
    constexpr auto str1 = "hewow"_cs;
    constexpr auto str2 = "hewow"_cs;
    constexpr auto str3 = "hewow2"_cs;

    static_assert(str1 == str2);
    static_assert(not(str1 == str3));

    ASSERT_TRUE(str1 == str2);
    ASSERT_FALSE(str1 == str3);
}

template <tmcpp::consteval_string A, tmcpp::consteval_string B>
constexpr auto
foo()
{
    return A == B;
}

TEST(consteval_string, can_be_used_as_nttp)
{
    constexpr auto str1 = "label1"_cs;
    constexpr auto str2 = "label2"_cs;

    static_assert(not foo<str1, str2>());
}
