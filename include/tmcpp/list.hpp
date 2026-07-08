#pragma once

#include <tuple>
#include <type_traits>

namespace tmcpp
{

template <typename... Types> struct list
{
    static constexpr auto size = sizeof...(Types);
    static constexpr auto is_empty = size == 0;

    // NOTE: users might have to write 'typename List::template at<INDEX>',
    // which is a little verbose.
    // TODO: consider converting this to a function so a user can write
    // 'List::at<INDEX>()' which is a little nicer syntax
    template <std::size_t I>
    using at = std::tuple_element_t<I, std::tuple<Types...>>;

    // TODO: idk if this is rlly needed
    template <typename T>
    consteval static auto
    contains()
    {
        return std::disjunction<std::is_same<T, Types>...>::value;
    }
};

};  // namespace tmcpp
