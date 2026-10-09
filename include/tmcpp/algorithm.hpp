#pragma once

#include "tmcpp/concepts.hpp"
#include "tmcpp/list.hpp"
#include "tmcpp/multilambda.hpp"

#include <type_traits>
#include <utility>

namespace tmcpp
{

// TODO: sometimes, we want to pass predicates/metafns to metafunctions here
// which accept nttps and/or typenames. currently, the user must wrap an nttp
// in a std::integral_constant<> in order to use the algorithms here. find out
// if it is possible to allow passing metafunctions (to algorithms such as
// find_type_if<>) which accept nttp, which would simplify user code

// type returned by search algorithms when no satisfactory types were found.
// note that void cannot be used since the caller may want to search for a void
// type in their list. having this distinct type allows the caller to
// differentiate those cases
struct not_found final
{
};

// for convenience
template <typename T> using is_not_found = std::is_same<T, not_found>;
template <typename T> constexpr auto is_not_found_v = is_not_found<T>::value;
//

// renames a template type containing some type parameters Ts into another
// template type containing Ts
template <ListLike From, template <typename...> typename To>
using rename = std::invoke_result_t<
    decltype([]<template <typename...> typename FromTemplate, typename... Ts>(
                 FromTemplate<Ts...>) -> To<Ts...> {}),
    From>;

// useful to succinctly define fold expression in concatenate implementation
template <typename... Ts, typename... Us>
consteval auto
operator+(list<Ts...>, list<Us...>) -> list<Ts..., Us...>
{
}

// given multiple typelist<T...> arguments, yields an alias for a single
// typelist<T...> which contains every type argument from each given typelist<>
template <ListLike... Lists>
using concatenate = std::remove_cvref_t<decltype((std::declval<Lists>() + ...
                                                  + std::declval<list<>>()))>;

// removes duplicates by keeping the first instance of a type, decided by
// Comparator
//
// NOTE: this looks like a mess but its straightforward. it works by
// concatenating a bunch of lists together, one for each element in List. for
// each type in List, it checks if any earlier index exists which contains a
// type that compares equal to the current type: if so, yield an empty list (so
// wont be included in final list); if not, then yield a list with the current
// element. the concatenation will flatten all these sub-lists (which are
// either empty of have 1 element) into a single list
//
// TODO: make a concept that checks the Comparator is valid for all
// pairs/combinations of types in the list
//
// TODO: we need a concept to check that all combinations/pairs of
// types in the list are valid for the comparator (or at least all the pairs we
// are going to check in the body). additionally, it seems clang crashes when
// we try to constrain the lambda in the decltype().
template <ListLike List, auto Comparator>
using remove_duplicates_if = std::invoke_result_t<
    decltype([]<std::size_t... I, typename... Ts>(std::index_sequence<I...>,
                                                  list<Ts...>)
                 -> concatenate<std::conditional_t<
                     []<std::size_t... J>(std::index_sequence<J...>)
                     {
                         // we use an alias here since using Ts directly will
                         // prematurely expand it with the nearest '...'
                         // operator
                         using T = Ts;
                         return (
                             Comparator.template operator()<T, at<J, List>>()
                             or ...);
                     }(std::make_index_sequence<I>{}),
                     list<>,
                     list<at<I, List>>>...> {}),
    std::make_index_sequence<size_v<List>>,
    List>;

// for convenience. elements are unique based on type
template <ListLike List>
using remove_duplicates
    = remove_duplicates_if<List,
                           []<typename T, typename U>
                           { return std::is_same_v<T, U>; }>;

// returns a list<> where each element is the corresponding element in List
// after having Fn applied to it, using the library's definition of
// 'type function application'
//
// NOTE:must have different name than 'transform' since type aliases
// do not participate in overload resolution, sadly
template <ListLike List, auto Fn>
    requires UnaryTypeFunctionForList<Fn, List>
using transform = std::invoke_result_t<
    decltype([]<typename... Types>(list<Types...>)
                 -> list<decltype(Fn.template operator()<Types>())...> {}),
    List>;

// returns the first type in the list, or the given default type if list is
// empty
// NOTE: we cant just use std::conditional_t<> since we need short-circuit
// evaluation, ie at<0, List> must only be evaluated for non-empty lists
// NOTE: we return an instance of std::type_identity since it doesnt construct
// a value of the type. then we extract the type after
// std::invoke_result_t. this means Default or first List element
// can be non-default-constructible.
template <ListLike List, typename Default>
using front_or = std::invoke_result_t<decltype([]{
    if constexpr (is_empty_v<List>) {
        return std::type_identity<Default>{};
    } else {
        return std::type_identity<at<0, List>>{};
    }
})>::type;

// yields a list<> containing all types in List which satisfy Predicate
template <ListLike List, auto Predicate>
    requires UnaryTypePredicateForList<Predicate, List>
using filter = std::invoke_result_t<
    decltype([]<typename... Ts>(list<Ts...>)
                 ->concatenate<
                     std::conditional_t<Predicate.template operator()<Ts>(),
                                        list<Ts>,
                                        list<>>...>{}),
    List>;

// returns the first type in given typelist that satisfies the given predicate,
// or returns special 'not found' type if no such types exist in the list
// TODO: i think this algorithm should return an empty list if there are no
// types that satisfy the predicate (instead of not_found). this might make it
// easier to chain algorithms using the result of find_if<>, without needing to
// make a special case to check not_found
template <ListLike List, auto Predicate>
    requires UnaryTypePredicateForList<Predicate, List>
using find_if = front_or<filter<List, Predicate>, not_found>;

// returns a list<> of std::integral_constant<std::size_t, I> where each I is
// an index into List such that the type at that index satisfies the predicate
//
// TODO: this implementation uses similar code to filter<>; perhaps we can
// implement one of these algorithms in terms of the other ?
// TODO: currently, the lambda takes 2 parameters since that makes it easier to
// access the List types; however, its probably possible to just take the index
// parameter and access the elements of the list via tmcpp::at<I>, but thats a
// lil more verbose
template <ListLike List, auto Predicate>
    requires UnaryTypePredicateForList<Predicate, List>
using filter_index = std::invoke_result_t<
    decltype([]<std::size_t... I, typename... Ts>(std::index_sequence<I...>,
                                                  list<Ts...>)
                 ->concatenate<std::conditional_t<
                     Predicate.template operator()<Ts>(),
                     list<std::integral_constant<std::size_t, I>>,
                     list<>>...>{}),
    std::make_index_sequence<size_v<List>>,
    List>;

// returns std::integral_constant<std::size_t, I> where I is the first index of
// the list containing a type satisfying the predicate. returns not_found if no
// such type exists in the list
template <ListLike List, auto Predicate>
    requires UnaryTypePredicateForList<Predicate, List>
using find_index_if = front_or<filter_index<List, Predicate>, not_found>;

};  // namespace tmcpp
