#pragma once

#include "tmcpp/concepts.hpp"

#include <tuple>

// we have to forward declare the typelist struct since we have to declare the
// std::tuple_lemeent specialization outside any namespace (im pretty sure)
namespace tmcpp
{

template <typename... T> struct typelist;

// needed for tuple_element specialization below
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

};  // namespace tmcpp

// TODO: for next time: we will need to rewrite all our code that operates on
// std::tuple<> types and replace it with typelist<>. we will need to rewrite
// make_unique_tuple<> to work on typelists, and implement a replacement for
// std::tuple_size_v<> to work with typelist. then replace all the code that
// uses the std primitives
//
// TODO: UPDATE: (to above todo): ok i specialized std::tuple_size below, so
// that we dont have to rewrite our other code that relies on using
// std::tuple<> for typelists
//
// TODO: UPDATE (to above todo): ok now i think its best we create own own
// versions of std::tuple*<> utilities to work on typelist. i just need to
// implement tuple_cat() for typelist

#if 0
//
// custom specialization so typelist can work with std::tuple_element
// this should allow us to just work with typelist<> whenever we need to
// instantiate any types in our decltype(lambda()) hacks below. by avoiding
// std::tuple<>, we can instantiate typelists with void as one of its types
template <std::size_t I, typename... T>
struct std::tuple_element<I, regi::mputils::typelist<T...>>
{
    using typelist_as_tuple
        = regi::mputils::rename<regi::mputils::typelist<T...>, std::tuple>;
    using type = std::tuple_element_t<I, typelist_as_tuple>;
};

template <class... Types>
struct std::tuple_size<regi::mputils::typelist<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{
};
//
#endif

namespace tmcpp
{

template <typename... Lambdas> struct multilambda : Lambdas...
{
    using Lambdas::operator()...;
};
// deduction guide to avoid specifying a ctor which might copy the lambdas
template <typename... Lambdas>
multilambda(Lambdas...) -> multilambda<Lambdas...>;

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

template <typename... Types> struct typelist
{
    // consteval static auto
    // size()
    // {
    //     return sizeof...(Types);
    // }

    static constexpr auto size = sizeof...(Types);

    consteval static auto
    is_empty()
    {
        return size == 0;
    }

    // TODO: idk if this is rlly needed
    template <typename T>
    consteval static auto
    contains()
    {
        return std::disjunction<std::is_same<T, Types>...>::value;
    }
};

// replacement for std::tuple_element_t<> for typelist<>
template <std::size_t I, typename Typelist>
using typelist_type_at = std::tuple_element_t<I, rename<Typelist, std::tuple>>;

// useful to succintly define fold expression in typelist_cat
template <typename... Ts, typename... Us>
consteval auto
operator+(typelist<Ts...>, typelist<Us...>)
{
    return typelist<Ts..., Us...>{};
}

// checks that a given template parameter is a typelist<> with any template
// args inside
// TODO: i would like to move this concept to concepts.hpp but we need the
// declaration of struct typelist<>, which occurs in this header. idk the best
// way to reconcile this...
template <typename T>
concept is_typelist
    = requires { []<typename... Us>(typelist<Us...>) {}(std::declval<T>()); };

// given multiple typelist<T...> arguments, yields an alias for a single
// typelist<T...> which contains every type argument from each given typelist<>
template <typename... Typelists>
    requires(is_typelist<Typelists> and ...)
using typelist_cat = decltype((Typelists{} + ... + typelist<>{}));

// https://stackoverflow.com/questions/55941964/how-to-filter-duplicate-types-from-tuple-c
// only keeps the last remaining duplicate element of the tuple
template <template <typename...> typename Comparator,
          typename T,
          typename... Rest>
    requires(concepts::comparator_metafunction<Comparator, T, Rest> and ...)
consteval auto
make_unique_typelist_if_impl(typelist<T, Rest...>)
{
    if constexpr ((Comparator<T, Rest>::value or ...))
    {
        return make_unique_typelist_if_impl<Comparator>(typelist<Rest...>{});
    }
    else
    {
        if constexpr (sizeof...(Rest) > 0)
        {
            using remaining
                = decltype(make_unique_typelist_if_impl<Comparator>(
                    typelist<Rest...>{}));
            return typelist_cat<typelist<T>, remaining>{};
        }
        else
        {
            return typelist<T>{};
        }
    }
}

// same as make_unique_tuple but two types T, U in TupleT are considered equal
// if Comparator<T, U>::value is true
template <template <typename...> typename Comparator, typename Typelist>
using make_unique_typelist_if
    = decltype(make_unique_typelist_if_impl<Comparator>(Typelist{}));

// for convenience
template <typename Typelist>
using make_unique_typelist = make_unique_typelist_if<std::is_same, Typelist>;

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
template <template <typename...> typename Predicate, typename Typelist>
    requires(is_typelist<Typelist>)
            using find_type_if
            = decltype([]<typename... Types>(typelist<Types...>)
                           requires(
                               concepts::predicate_metafunction_for<Predicate,
                                                                    Types>
                               and ...)
            {
                // we use conditional_t<> for brevity to select if each type
                // should be added to the list of types satisfying the
                // predicate
                using found_list = typelist_cat<typename std::conditional_t<
                    Predicate<Types>::value, typelist<Types>, typelist<>>...>;
                if constexpr (found_list::size == 0)
                {
                    // nothing found; return void
                    return;
                }
                else
                {
                    // found at least 1, so return the first one.
                    // return std::tuple_element_t<0, found_list>{};
                    return typelist_type_at<0, found_list>{};
                }
            }(Typelist{}));

// same as normal version, but accepts a quoted meta function as the predicate
template <concepts::quoted_metafunction QuotedPredicate, typename Typelist>
using find_type_if_q = find_type_if<QuotedPredicate::template fn, Typelist>;

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
    = decltype([]<typename... Types>(typelist<Types...>)
                   requires(concepts::metafunction_for<Fn, Types> and ...)
               { return typelist<typename Fn<Types>::type...>{}; }(
                   Typelist{}));

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

};  // namespace tmcpp
