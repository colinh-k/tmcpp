#pragma once

#include <type_traits>

namespace tmcpp
{

template <typename... Types> struct list
{
    static constexpr auto size = sizeof...(Types);
    static constexpr auto is_empty = size == 0;

    // TODO: idk if this is rlly needed
    template <typename T>
    consteval static auto
    contains()
    {
        return std::disjunction<std::is_same<T, Types>...>::value;
    }
};

};  // namespace tmcpp
