#pragma once

#include <concepts>

#include "tmcpp/list.hpp"
#include "tmcpp/multilambda.hpp"

namespace tmcpp
{

namespace concepts
{

// constrains T to be a template type matching Template, but T and Template may
// have different template arguments
// eg TemplateOf<std::tuple<int, float>, std::tuple> is legal
template <typename T, template <typename...> typename Template>
concept TemplateOf
    = std::invoke_result_t<decltype(multilambda{
                               []<typename... U>(const Template<U...> &)
                               { return std::true_type{}; },
                               [](const auto &) { return std::false_type{}; },
                           }),
                           T>::value;

// metafunctions must define a template alias member 'invoke', and must accept
// any arity of arguments Args
template <typename T, typename... Args>
concept MetafunctionFor = requires { typename T::template invoke<Args...>; };

// checks that T can be invoked with all types in List, one-at-a-time
template <typename T, typename List>
concept UnaryMetafunctionForList
    = ListLike<List> and not tmcpp::is_empty_v<List>
      and ([]<typename... Us>(list<Us...>)
           { return (MetafunctionFor<T, Us> and ...); }(List{}));

// nttp version to work with any template callable object
template <auto T, typename... Args>
concept MetafunctionObjectFor = requires { T.template operator()<Args...>(); };

// TODO: note that passing an invalid metafunction object with an empty list
// will NOT be caught by this concept. the issue is that the pack expansion
// would be empty, so anything passed as T would succeed. but there still might
// be a poor-quality compile error later on when the illegal T is used in the
// body of whatever template using this concept. the solution is to require
// List to be nonempty. however, for some algorithms like transform, it is
// perfectly valid to transform an empty list (the result is just an empty
// list), so we cannot constrain List to be nonempty here.
// the same issue exists for other concepts like UnaryMetafunctionForList
// NOTE: to save us from bad template errors later, im going to enforce
// nonempty list for now
template <auto T, typename List>
concept UnaryMetafunctionObjectForList
    = ListLike<List> and not tmcpp::is_empty_v<List>
      and ([]<typename... Us>(list<Us...>)
           { return (MetafunctionObjectFor<T, Us> and ...); }(List{}));

// a type which accepts a single template parameter and yields a bool. useful
// to pass as predicates to tmp algorithms
template <typename T, typename Arg>
concept UnaryPredicateFor = requires {
    { T::template invoke<Arg>::value } -> std::convertible_to<bool>;
};

// for convenience if the predicate arguments are already in a tmcpp::list<>
template <typename T, typename List>
concept UnaryPredicateForList
    = ListLike<List> and not tmcpp::is_empty_v<List>
      and ([]<typename... Us>(list<Us...>)
           { return (UnaryPredicateFor<T, Us> and ... and true); }(List{}));

template <auto T, typename Arg>
concept UnaryPredicateObjectFor = requires {
    { T.template operator()<Arg>() } -> std::convertible_to<bool>;
};

// for convenience if the predicate arguments are already in a tmcpp::list<>
template <auto T, typename List>
concept UnaryPredicateObjectForList
    = ListLike<List> and not tmcpp::is_empty_v<List>
      and ([]<typename... Us>(list<Us...>)
           { return (UnaryPredicateObjectFor<T, Us> and ... and true); }(
               List{}));

template <typename T, typename Arg1, typename Arg2>
concept BinaryPredicateFor = requires {
    { T::template invoke<Arg1, Arg2>::value } -> std::convertible_to<bool>;
};

template <auto T, typename Arg1, typename Arg2>
concept BinaryPredicateObjectFor = requires {
    { T.template operator()<Arg1, Arg2>() } -> std::convertible_to<bool>;
};

// TODO: i want to write a 'concept comparator_metafunction_for', but im not
// exactly sure how to write a concept for a metafunction that takes 2 args,
// for each pair of args in a list. we should write one later for consistency

};  // namespace concepts

};  // namespace tmcpp
