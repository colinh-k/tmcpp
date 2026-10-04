#pragma once

#include <concepts>

#include "tmcpp/multilambda.hpp"

#include "tmcpp/list.hpp"

namespace tmcpp
{

namespace concepts
{

// constrains T to be a template type matching TargetT, but T and TargetT may
// have different template arguments
// eg is_template_of<std::tuple<int, float>, std::tuple> is legal
template <typename T, template <typename...> typename Template>
concept TemplateOf
    = std::invoke_result_t<decltype(multilambda{
                               []<typename... U>(const Template<U...> &)
                               { return std::true_type{}; },
                               [](const auto &) { return std::false_type{}; },
                           }),
                           T>::value;

template <typename T>
concept ListLike = TemplateOf<T, list>;

// metafunctions must define a template alias member 'invoke', and must accept
// any arity of arguments Args
template <typename T, typename... Args>
concept MetafunctionFor = requires { typename T::template invoke<Args...>; };

// checks that T can be invoked with all types in List, one-at-a-time
template <typename T, typename List>
concept UnaryMetafunctionForList
    = ListLike<List>
      and ([]<typename... Us>(list<Us...>)
           { return (MetafunctionFor<T, Us> and ...); }(List{}));

// a type which accepts a single template parameter and yields a bool. useful
// to pass as predicates to tmp algorithms
template <typename T, typename Arg>
concept UnaryPredicateFor = requires {
    { T::template invoke<Arg>::value } -> std::convertible_to<bool>;
};
#if 0
concept UnaryPredicateFor = ((MetafunctionFor<T, Args> and requires {
                                 {
                                     T::template invoke<Args>::value
                                 } -> std::convertible_to<bool>;
                             }) and ...);
#endif

// for convenience if the predicate arguments are already in a tmcpp::list<>
template <typename T, typename List>
concept UnaryPredicateForList
    = ListLike<List>
      and ([]<typename... Us>(list<Us...>)
           { return (UnaryPredicateFor<T, Us> and ... and true); }(List{}));

// L is a tuple of types which will be passed to the metafunction
// TODO: this concept is not very well-posed; we need a better way to check
// that every argument we intend to pass to the quoted function is valid. im
// not rlly sure there is a good way to determine a 'valid' result, since any
// type is technically valid. all we need to check is that fn is a template
// member of T, that yields a type. think about it some more bc this may be
// what we have now... idk
// TODO: i arbitrarily use 'int' as the fn<> parameter since this will make
// sure fn<> can accept exactly one parameter (i think). this will give strange
// error messages if user supplies a non-conforming type, which is bad for the
// user experience
template <typename T>
concept quoted_metafunction = requires { typename T::template fn<int>; };

// binary metafunction intended to check if two types are equal
// template <template <typename...> typename T, typename Arg1, typename Arg2>
// concept comparator_metafunction = requires {
//     { T<Arg1, Arg2>::value } -> std::convertible_to<bool>;
// };

template <typename T, typename Arg1, typename Arg2>
concept BinaryPredicateFor = requires {
    { T::template invoke<Arg1, Arg2>::value } -> std::convertible_to<bool>;
};

// TODO: i want to write a 'concept comparator_metafunction_for', but im not
// exactly sure how to write a concept for a metafunction that takes 2 args,
// for each pair of args in a list. we should write one later for consistency

};  // namespace concepts

};  // namespace tmcpp
