#pragma once

#include "tmcpp/concepts.hpp"
#include "tmcpp/list.hpp"

#include <tuple>
#include <type_traits>

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
struct not_found_type
{
};

// replacement for std::tuple_element_t<> for typelist<>
template <std::size_t I, typename Typelist>
using at = std::tuple_element_t<I, rename<Typelist, std::tuple>>;

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

// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-tuple-c
// only keeps the last remaining duplicate element of the tuple
template <template <typename...> typename Comparator,
          typename T,
          typename... Rest>
    requires(concepts::comparator_metafunction<Comparator, T, Rest> and ...)
consteval auto
make_unique_typelist_if_impl(list<T, Rest...>)
{
    if constexpr ((Comparator<T, Rest>::value or ...))
    {
        return make_unique_typelist_if_impl<Comparator>(list<Rest...>{});
    }
    else
    {
        if constexpr (sizeof...(Rest) > 0)
        {
            using remaining
                = decltype(make_unique_typelist_if_impl<Comparator>(
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
template <template <typename...> typename Comparator, typename Typelist>
using make_unique_typelist_if
    = decltype(make_unique_typelist_if_impl<Comparator>(Typelist{}));

// for convenience. elements are unique based on type
template <typename Typelist>
using make_unique_typelist = make_unique_typelist_if<std::is_same, Typelist>;

// yields a list<> containing all types in List which satisfy Predicate
template <typename Predicate, typename List>
using filter = decltype([]<typename... T>(list<T...>){
    return concatenate< std::conditional_t< Predicate::template value<T>, list<T>, list<> >... >{};
}(std::declval<List>()));

// returns the first type in given typelist that satisfies the given predicate,
// or returns void if none found
// NOTE: Typelist is a tuple of types NOTE: the
// predicate parameter expects to recieve type tempalte parameters (ie NOT
// non-type template parameters). to use a predicate that expects an nttp, use
// the structural_constant wrapper below (see test cases for example)
// TODO: is there some way we can overload this alias (using requires() clauses
// ?) so that we can accept usual predicates and also predicates which are
// quoted metafunctions ? (idk if its even legal to overload an alias). this
// would make it easier for the user to construct predicates (ie using
// bind_front<>())
#if 1
template <typename Predicate, typename Typelist>
    requires(concepts::is_template_of<Typelist, list>
             and concepts::predicate_metafunction_for_list<Predicate, Typelist, list>)
            using find_type_if
            = decltype([]<typename... Types>(list<Types...>)
            {
                // we use conditional_t<> for brevity to select if each type
                // should be added to the list of types satisfying the
                // predicate
                using found_list = concatenate<typename std::conditional_t<
                    Predicate::template value<Types>, list<Types>, list<>>...>;
                if constexpr (found_list::is_empty)
                {
                    // nothing found; return void
                    return;
                }
                else
                {
                    // found at least 1, so return the first one.
                    // return std::tuple_element_t<0, found_list>{};
                    return at<0, found_list>{};
                }
            }(Typelist{}));
#else
template <typename Predicate, typename List>
    requires(concepts::is_template_of<List, list>
             and concepts::
                 predicate_metafunction_for_list<Predicate, List, list>)
using find_type_if = void;
#endif

// same as normal version, but accepts a quoted meta function as the predicate
// template <concepts::quoted_metafunction QuotedPredicate, typename Typelist>
// using find_type_if_q = find_type_if<QuotedPredicate::template fn, Typelist>;

// TODO: in most of these metafunctions, we instantiate a tuple like
// tuple<...>{}. in doing so, we implicitly require that none of the types in
// that tuple are void; if they are, a compiler error is generated. SOLUTION:
// to fix this issue, we instantiate typelist<...>{} which places no
// restriction on the type args when instantiated. then, we conver the typelist
// to a tuple without instantiating
// TODO: we probably dont need to rename<> to std::tuple anymore, since
// typelist<> can handle most lists of types the user expects
template <template <typename...> typename Fn, typename Typelist>
using transform
    = decltype([]<typename... Types>(list<Types...>)
                   requires(concepts::metafunction_for<Fn, Types> and ...)
               { return list<typename Fn<Types>::type...>{}; }(Typelist{}));

// takes a template metafn that accepts multiple template arguments
// FnT and some of those arguments Args. the member fn is a new
// template which accepts the remaining arguments Remaining which are passed as
// the arguments to FnT after Args, ie as FnT<Args..., Remaining...>
// TODO: using the bound metafunction fn requires syntax like
// 'bind<P,U>::template fn'. instead, we could have an alias called 'invoke'
// which automatically converts a bound metafn to its inner fn member.... idk
// look at boost mp11 for inspo
template <template <typename...> typename FnT, typename... Args>
struct bind_front
{
    // NOTE: idk why the following does not work, but creating a nested struct
    // (as below) does work
    //
    // template <typename... Remaining> using fn = FnT<Args..., Remaining...>;

    template <typename... Remaining> struct apply
    {
        using type = FnT<Args..., Remaining...>;
    };

    template <typename... Remaining>
    using fn = typename apply<Remaining...>::type;
};

// call a quoted metafn Fn with arguments T
template <typename Fn, typename... T>
    requires(concepts::quoted_metafunction<Fn>)
using invoke = typename Fn::template fn<T...>;

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
