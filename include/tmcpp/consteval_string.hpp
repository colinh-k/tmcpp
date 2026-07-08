#pragma once

#include <algorithm>
#include <array>
#include <string>

namespace tmcpp
{

// a string suitable for compile-time-only processing, and can be used as a
// nttp
template <std::size_t N> struct consteval_string
{
    std::array<char, N> data{};

    // TODO: would move semantics help when transferring the char buffer to
    // this object ?
    consteval consteval_string(char const (&str)[N])
    {
        std::copy_n(std::begin(str), N, std::begin(data));
    }

    template <std::size_t M>
    consteval auto
    operator==(const consteval_string<M> &other) const
    {
        // NOTE: we must use 'if constexpr' (instead of a single, compound
        // boolean expression) since operator== is not defined for std::array
        // with different sizes
        if constexpr (M == N)
        {
            return this->data == other.data;
        }
        else
        {
            return false;
        }
    }
};

// TODO: the ""_L operator should be moved to the regi project; this generic
// project should define a generic udl
// TODO: should literals namespace be in a separate file ?
namespace literals
{

// '_cs' for 'consteval_string'
template <consteval_string Str>
consteval auto
operator""_cs()
{
    return Str;
}

};  // namespace literals

};  // namespace tmcpp
