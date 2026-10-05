#pragma once

#include "tmcpp/concepts.hpp"
#include "tmcpp/list.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

// NOTE: the algorithms ending in _with (eg transform_with vs transform)
// perform the same operation as their other counterparts, except they take the
// metafunction template argument as an nttp. we must introduce a new name
// since type aliases do not participate in overload resolution. this leads to
// awkward names like find_index_if_with

// TODO: the algorithms should NOT return void to indicate a null/empty return
// value, since perhaps the user wants (eg) find_if<IsVoidPredicate,
// ListWithVoid> to return the first instance of void in a given list. maybe
// create a 'not found' type which can be returned instead

namespace tmcpp
{

// TODO: sometimes, we want to pass predicates/metafns to metafunctions here
// which accept nttps and/or typenames. currently, the user must wrap an nttp
// in a std::integral_constant<> in order to use the algorithms here. find out
// if it is possible to allow passing metafunctions (to algorithms such as
// find_type_if<>) which accept nttp, which would simplify user code

// TODO: i should add requires clauses/concepts to each template parameter
// where appropriate for better error messages

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

template <concepts::ListLike List>
using is_empty = std::bool_constant<List::is_empty>;
template <concepts::ListLike List> constexpr bool x = is_empty<List>{};

template <std::size_t I, concepts::ListLike List>
using at = typename List::template at<I>;
//

// renames a template type containing some type parameters Ts into another
// template type containing Ts
template <typename From, template <typename...> typename To>
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
template <concepts::ListLike... Lists>
using concatenate = std::remove_cvref_t<decltype((std::declval<Lists>() + ...
                                                  + std::declval<list<>>()))>;

// return list<Ts..., U> where Ts are the types in List provided U is not in Ts
// (as decided by the comparator); otherwise return List
template <typename Comparator, concepts::ListLike List, typename U>
using append_if_unique = std::invoke_result_t<
    decltype([]<typename... Ts>(list<Ts...>)
                 -> std::conditional_t<
                     (Comparator::template invoke<Ts, U>::value or ...
                      or false),
                     List,
                     list<Ts..., U>>
             // TODO: clang crashes if this requires clause is un-commented:
             // requires(concepts::BinaryPredicateFor<Comparator, Ts, U>
             // and ... and true)
             {}),
    List>;

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
template <typename Comparator, concepts::ListLike List>
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
                             Comparator::template invoke<T, at<J, List>>::value
                             or ...);
                     }(std::make_index_sequence<I>{}),
                     list<>,
                     list<at<I, List>>>...> {}),
    std::make_index_sequence<List::size>,
    List>;

// nttp version
//
// TODO: this and the regular version both dont have concepts constraining the
// template args. we need a concept to check that all combinations/pairs of
// types in the list are valid for the comparator (or at least all the pairs we
// are going to check in the body). additionally, it seems clang crashes when
// we try to constrain the lambda in the decltype().
template <auto Comparator, concepts::ListLike List>
using remove_duplicates_if_with = std::invoke_result_t<
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
    std::make_index_sequence<List::size>,
    List>;

// for convenience. elements are unique based on type
template <concepts::ListLike List>
using remove_duplicates
    = remove_duplicates_if_with<[]<typename T, typename U>
                                { return std::is_same_v<T, U>; },
                                List>;

// returns a list<> where each element is the corresponding element in List
// after having Fn applied to it, using the library's definition of
// 'metafunction application'
template <typename Fn, concepts::ListLike List>
    requires(concepts::UnaryMetafunctionForList<Fn, List>)
using transform = std::invoke_result_t<
    decltype([]<typename... Types>(list<Types...>)
                 -> list<typename Fn::template invoke<Types>...> {}),
    List>;

// nttp version. allows defining a template metafunction as a lambda inline
// instead of pre-defining a metafunction struct/class with an 'invoke' member
// outside the call site's scope
//
// NOTE:must have different name than 'transform' since type aliases
// do not participate in overload resolution, sadly
template <auto Fn, concepts::ListLike List>
    requires(concepts::UnaryMetafunctionObjectForList<Fn, List>)
using transform_with = std::invoke_result_t<
    decltype([]<typename... Types>(list<Types...>)
                 -> list<decltype(Fn.template operator()<Types>())...> {}),
    List>;

// returns the first type in the list, or the given default type if list is
// empty
// NOTE: we cant just use std::conditional_t<> since we need short-circuit
// evaluation, ie at<0, List> must only be evaluated for non-empty lists
template <concepts::ListLike List, typename Default>
using front_or = std::invoke_result_t<decltype([]{
    if constexpr (List::is_empty) {
        return Default{};
    } else {
        return at<0, List>{};
    }
})>;

// yields a list<> containing all types in List which satisfy Predicate
template <typename Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateForList<Predicate, List>)
using filter = std::invoke_result_t<
    decltype([]<typename... Ts>(list<Ts...>)
                 -> concatenate<
                     std::conditional_t<Predicate::template invoke<Ts>::value,
                                        list<Ts>,
                                        list<>>...> {}),
    List>;

// nttp version
template <auto Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateObjectForList<Predicate, List>)
using filter_with = std::invoke_result_t<
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
template <typename Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateForList<Predicate, List>)
using find_if = front_or<filter<Predicate, List>, not_found>;

// nttp version
template <auto Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateObjectForList<Predicate, List>)
using find_if_with = front_or<filter_with<Predicate, List>, not_found>;

// returns a list<> of std::integral_constant<std::size_t, I> where each I is
// an index into List such that the type at that index satisfies the predicate
//
// TODO: this implementation uses similar code to filter<>; perhaps we can
// implement one of these algorithms in terms of the other ?
// TODO: currently, the lambda takes 2 parameters since that makes it easier to
// access the List types; however, its probably possible to just take the index
// parameter and access the elements of the list via typename List::template
// at<I>, but thats a lil more verbose
template <typename Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateForList<Predicate, List>)
using filter_index = std::invoke_result_t<
    decltype([]<std::size_t... I, typename... Ts>(std::index_sequence<I...>,
                                                  list<Ts...>)
                 -> concatenate<std::conditional_t<
                     Predicate::template invoke<Ts>::value,
                     list<std::integral_constant<std::size_t, I>>,
                     list<>>...> {}),
    std::make_index_sequence<List::size>,
    List>;

// nttp version
template <auto Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateObjectForList<Predicate, List>)
using filter_index_with = std::invoke_result_t<
    decltype([]<std::size_t... I, typename... Ts>(std::index_sequence<I...>,
                                                  list<Ts...>)
                 ->concatenate<std::conditional_t<
                     Predicate.template operator()<Ts>(),
                     list<std::integral_constant<std::size_t, I>>,
                     list<>>...>{}),
    std::make_index_sequence<List::size>,
    List>;

// returns std::integral_constant<std::size_t, I> where I is the first index of
// the list containing a type satisfying the predicate. returns not_found if no
// such type exists in the list
template <typename Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateForList<Predicate, List>)
using find_index_if = front_or<filter_index<Predicate, List>, not_found>;

// nttp version
template <auto Predicate, concepts::ListLike List>
    requires(concepts::UnaryPredicateObjectForList<Predicate, List>)
using find_index_if_with
    = front_or<filter_index_with<Predicate, List>, not_found>;

// turns a non-quoted metafunction into a quoted metafunction suitable to be
// passed to invoke<>. mainly for convenience
template <template <typename...> typename Fn> struct quote
{
    template <typename... ArgsT> using fn = Fn<ArgsT...>;
};

};  // namespace tmcpp

#if 0
fundamental algorithms:

transform
filter
concat
front
front_or
empty
#endif
