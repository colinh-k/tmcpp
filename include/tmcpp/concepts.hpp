#pragma once

#include <concepts>

#include "tmcpp/list.hpp"
#include "tmcpp/multilambda.hpp"

namespace tmcpp
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

// a type with a templated call operator that returns bool. useful to constrain
// algorithms that accept predicates
template <auto T, typename Arg>
concept UnaryPredicateObjectFor = requires {
    { T.template operator()<Arg>() } -> std::convertible_to<bool>;
};

// for convenience if the predicate arguments are already in a list-like
// structure
template <auto T, typename List>
concept UnaryPredicateObjectForList
    = ListLike<List> and not tmcpp::is_empty_v<List>
      and ([]<typename... Us>(list<Us...>)
           { return (UnaryPredicateObjectFor<T, Us> and ... and true); }(
               List{}));

template <auto T, typename Arg1, typename Arg2>
concept BinaryPredicateObjectFor = requires {
    { T.template operator()<Arg1, Arg2>() } -> std::convertible_to<bool>;
};

};  // namespace tmcpp
