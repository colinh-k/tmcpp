#pragma once

#include <tuple>

namespace tmcpp
{

template <typename... Types> struct list
{
    static constexpr auto size = sizeof...(Types);
    static constexpr auto is_empty = size == 0;

    // NOTE: users might have to write 'typename List::template at<INDEX>',
    // which is a little verbose.
    template <std::size_t I>
    using at = std::tuple_element_t<I, std::tuple<Types...>>;
};

};  // namespace tmcpp
