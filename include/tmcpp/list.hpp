#pragma once

#include <tuple>

namespace tmcpp
{

template <typename T>
concept ListLike = requires {
    // to be a type list, we must be able to deduce that T is actually a
    // template with an arbitrary number of template type parameters
    //
    // this is the main construct we use to work with any type-list-like type
    {
        []<template <typename...> typename Template, typename... Types>(
            Template<Types...>) {}(std::declval<T>())
    };
};

template <typename... Types> struct list
{
};

// for now, im following the stl convention of supplying normal and _v versions
// of type traits, but idk if the normal versions are useful, so i may rename
// them later

// convenience functions to query lists
template <ListLike List>
using size = std::invoke_result_t<
    decltype([]<template <typename...> typename Template, typename... Types>(
                 Template<Types...>) -> std::tuple_size<std::tuple<Types...>>
             {}),
    List>;
template <ListLike List> constexpr auto size_v = size<List>::value;

template <ListLike List>
using is_empty = std::bool_constant<size_v<List> == 0>;
template <ListLike List> constexpr auto is_empty_v = is_empty<List>::value;

template <std::size_t I, ListLike List>
using at = std::invoke_result_t<
    decltype([]<template <typename...> typename Template, typename... Types>(
                 Template<Types...>)
                 -> std::tuple_element_t<I, std::tuple<Types...>> {}),
    List>;

};  // namespace tmcpp
