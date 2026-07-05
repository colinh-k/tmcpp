#pragma once

#include <algorithm>
#include <array>
#include <string>

namespace tmcpp
{

template <std::size_t N> struct ConstexprString
{
    // TODO: would move semantics help when transferring the char buffer to
    // this object ?
    consteval ConstexprString(char const (&str)[N])
    {
        std::copy_n(std::begin(str), N, std::begin(data));
    }

    template <std::size_t M>
    consteval auto
    operator==(const ConstexprString<M> &other) const
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

    std::array<char, N> data{};
};

// TODO: should literals namespace be in a separate file ?
namespace literals
{

// '_L' for 'Label', which is the intended use case
template <ConstexprString Str>
consteval auto
operator""_L()
{
    return Str;
}

};  // namespace literals

};  // namespace regi
