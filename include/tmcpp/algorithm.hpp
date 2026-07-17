#pragma once

#include "tmcpp/concepts.hpp"
#include "tmcpp/list.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

// TODO: the algorithms should NOT return void to indicate a null/empty return
// value, since perhaps the user wants (eg) find_if<IsVoidPredicate,
// ListWithVoid> to return the first instance of void in a given list. maybe
// create a 'not found' type which can be returned instead

namespace tmcpp
{

// classic rename implementation
template <class A, template <class...> class B> struct rename_impl;

template <template <class...> class A, class... T, template <class...> class B>
struct rename_impl<A<T...>, B>
{
    using type = B<T...>;
};

template <class A, template <class...> class B>
using rename = typename rename_impl<A, B>::type;
//

// TODO: sometimes, we want to pass predicates/metafns to metafunctions here
// which accept nttps and/or typenames. currently, the user must wrap an nttp
// in a std::integral_constant<> in order to use the algorithms here. find out
// if it is possible to allow passing metafunctions (to algorithms such as
// find_type_if<>) which accept nttp, which would simplify user code

// TODO: i should add requires clauses/concepts to each template parameter
// where appropriate for better error messages

// TODO: since we typeically only want lists of bit ranges with unique ids,
// maybe just make a subclass that assumes it has unique ids. it can assert
// uniqueness at compile time, and simplify the interface for lookup by id
// (ie eliminate returning a tuple) since we know it either exists in the list
// or not (there can be no duplicates)

// type returned by search algorithms when no satisfactory types were found.
// note that void cannot be used since the caller may want to search for a void
// type in their list. having this distinct type allows the caller to
// differentiate those cases
// TODO: perhaps we should introduce tmcpp::is_not_found<T> for convenience to
// check if a return type from one of our algorithms is not_found
struct not_found final
{
};

#if 0
// replacement for std::tuple_element_t<> for typelist<>
template <std::size_t I, typename List>
    requires(concepts::is_template_of<List, list>)
using at = std::tuple_element_t<I, rename<List, std::tuple>>;
#endif

template <typename List>
    requires(concepts::is_template_of<List, list>)
using is_empty = std::bool_constant<List::is_empty>;

// useful to succintly define fold expression in typelist_cat
template <typename... Ts, typename... Us>
consteval auto
operator+(list<Ts...>, list<Us...>)
{
    return list<Ts..., Us...>{};
}

// given multiple typelist<T...> arguments, yields an alias for a single
// typelist<T...> which contains every type argument from each given typelist<>
template <typename... Typelists>
    requires(concepts::is_template_of<Typelists, list> and ...)
using concatenate = decltype((Typelists{} + ... + list<>{}));

// return list<Ts..., U> where Ts are the types in List provided U is not in Ts
// (as decided by the comparator); otherwise return List
template <typename Comparator, typename List, typename U>
using append_if_unique = decltype([]<typename... Ts>(list<Ts...>)
{
    return std::conditional_t<(Comparator::template invoke<Ts, U>::value or ... or false), List, list<Ts..., U>>{};
}(List{}));

// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-tuple-c
// only keeps the last remaining duplicate element of the tuple
// TODO: we MUST make sure this works for empty or single-element lists. i ran
// into issues trying to use it on empty lists earlier...
template <typename Comparator, typename T, typename... Rest>
    requires(concepts::comparator_metafunction_for<Comparator, T, Rest>
             and ...)
consteval auto
remove_duplicates_if_impl(list<T, Rest...>)
{
    if constexpr ((Comparator::template invoke<T, Rest>::value or ...))
    {
        return remove_duplicates_if_impl<Comparator>(list<Rest...>{});
    }
    else
    {
        if constexpr (sizeof...(Rest) > 0)
        {
            using remaining = decltype(remove_duplicates_if_impl<Comparator>(
                list<Rest...>{}));
            return concatenate<list<T>, remaining>{};
        }
        else
        {
            return list<T>{};
        }
    }
}

// same as make_unique_tuple but two types T, U in TupleT are considered equal
// if Comparator<T, U>::value is true
template <typename Comparator, typename List>
using remove_duplicates_if
    = decltype(remove_duplicates_if_impl<Comparator>(List{}));

// std::is_same but wrapped in an invokable metafunction appropriate for
// algorithms with comparators
struct is_same_comparator
{
    template <typename T, typename U> using invoke = std::is_same<T, U>;
};

// for convenience. elements are unique based on type
template <typename List>
using remove_duplicates = remove_duplicates_if<is_same_comparator, List>;

// returns a list<> where each element is the corresponding element in List
// after having Fn applied to it, using the library's definition of
// 'metafunction application'
template <typename Fn, typename List>
    requires(concepts::unary_metafunction_for_list<Fn, List, list>)
using transform
    = decltype([]<typename... Types>(list<Types...>)
               { return list<typename Fn::template invoke<Types>...>{}; }(
                   List{}));

// returns the first type in the list, or the given default type if list is
// empty
// NOTE: we cant just use std::conditional_t<> since we need short-circuit
// evaluation, ie at<0, List> must only be evaluated for non-empty lists
template <typename List, typename Default>
    requires(concepts::is_template_of<List, list>)
using front_or
    = decltype([](){
    if constexpr (List::is_empty) {
        return Default{};
    } else {
        return typename List::template at<0>{};
    }
}());

// yields a list<> containing all types in List which satisfy Predicate
template <typename Predicate, typename List>
using filter = decltype([]<typename... T>(list<T...>){
    return concatenate< std::conditional_t< Predicate::template invoke<T>::value, list<T>, list<> >... >{};
}(std::declval<List>()));

// returns the first type in given typelist that satisfies the given predicate,
// or returns special 'not found' type if no such types exist in the list
// TODO: i think this algorithm should return an empty list if there are no
// types that satisfy the predicate (instead of not_found). this might make it
// easier to chain algorithms using the result of find_if<>, without needing to
// make a special case to check not_found
template <typename Predicate, typename List>
    requires(concepts::is_template_of<List, list>
             and concepts::
                 predicate_metafunction_for_list<Predicate, List, list>)
using find_if = front_or<filter<Predicate, List>, not_found>;

// returns a list<> of std::integral_constant<std::size_t, I> where each I is
// an index into List such that the type at that index satisfies the predicate
//
// TODO: this implementation uses similar code to filter<>; perhaps we can
// implement one of these algorithms in terms of the other ?
// TODO: currently, the lambda takes 2 parameters since that makes it easier to
// access the List types; however, its probably possible to just take the index
// parameter and access the elements of the list via typename List::template
// at<I>, but thats a lil more verbose
template <typename Predicate, typename List>
    requires(concepts::is_template_of<List, list>
             and concepts::
                 predicate_metafunction_for_list<Predicate, List, list>)
using filter_index = decltype([]<std::size_t... I, typename... T>(std::index_sequence<I...>, list<T...>){
    return concatenate< std::conditional_t< Predicate::template invoke<T>::value, list<std::integral_constant<std::size_t, I>>, list<> >... >{};
}(std::make_index_sequence<List::size>(), std::declval<List>()));
;

// returns std::integral_constant<std::size_t, I> where I is the first index of
// the list containing a type satisfying the predicate. returns not_found if no
// such type exists in the list
template <typename Predicate, typename List>
    requires(concepts::is_template_of<List, list>
             and concepts::
                 predicate_metafunction_for_list<Predicate, List, list>)
using find_index_if = front_or<filter_index<Predicate, List>, not_found>;

// for now, im removing bind_front<> since idk how to fix the issues were
// facing rn
#if 0
// takes a template metafn that accepts multiple template arguments
// FnT and some of those arguments Args. the member fn is a new
// template which accepts the remaining arguments Remaining which are passed as
// the arguments to FnT after Args, ie as FnT<Args..., Remaining...>
// TODO: using the bound metafunction fn requires syntax like
// 'bind<P,U>::template fn'. instead, we could have an alias called 'invoke'
// which automatically converts a bound metafn to its inner fn member.... idk
// look at boost mp11 for inspo
template <typename Fn, typename... Args> struct bind_front
{
    // NOTE: idk why the following does not work, but creating a nested struct
    // (as below) does work
    //
    template <typename... Remaining>
    using invoke = typename Fn::template invoke<Args..., Remaining...>;

    // template <typename... Remaining> struct apply
    // {
    //     using type = Fn<Args..., Remaining...>;
    // };
    //
    // template <typename... Remaining>
    // using fn = typename apply<Remaining...>::type;
};

template <typename Fn, typename... T> struct invoke_impl
{
    using type = typename Fn::template invoke<T...>;
};

// call a quoted metafn Fn with arguments T
template <typename Fn, typename... T> using invoke = invoke_impl<Fn, T...>;
// using invoke = typename Fn::template invoke<T...>;
#endif

// turns a non-quoted metafunction into a quoted metafunction suitable to be
// passed to invoke<>. mainly for convenience
template <template <typename...> typename Fn> struct quote
{
    template <typename... ArgsT> using fn = Fn<ArgsT...>;
};

// unsure if we need this anymore
#if 0
// returns a tuple of tuples, where tuple_i contains all the types in
// TypesWithId pack with the same id value
// TODO: maybe it would be better to supply a predicate/comparator which can be
// used to tell if two types are the same by id; this way, we dont require the
// provided types to have an 'id' field, which means the user can use it on
// types with fields with other names
// TODO: add a concept/requires() clause to constrain the template param
template <typename... TypesWithId>
consteval static auto
group_by_id()
{
    // move all types with the same id into their own tuple. this may result in
    // duplicates, which are removed below
    constexpr auto grouped_with_duplicates = std::tuple(
        [](auto target)
        {
            using TargetType = decltype(target);
            return std::tuple_cat(
                [](auto current)
                {
                    using CurrentType = decltype(current);
                    if constexpr (CurrentType::id == TargetType::id)
                    {
                        return std::tuple<CurrentType>{};
                    }
                    else
                    {
                        return std::tuple<>{};
                    }
                }(TypesWithId{})...);
        }(TypesWithId{})...);

    return make_unique_typelist_if_impl(grouped_with_duplicates);
}
#endif

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
