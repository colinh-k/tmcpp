#pragma once

#include <concepts>

#include "tmcpp/multilambda.hpp"

namespace tmcpp
{

namespace concepts
{

// constrains T to be a template type matching TargetT, but T and TargetT may
// have different template arguments
// eg is_template_of<std::tuple<int, float>, std::tuple> is legal
template <typename T, template <typename...> typename TargetT>
concept is_template_of = decltype(multilambda{
    []<typename... U>(const TargetT<U...> &) { return std::true_type{}; },
    [](const auto &) { return std::false_type{}; },
}(std::declval<T>()))::value;

// metafunctions must define a template alias member 'invoke', and must accept
// any arity of arguments Args
template <typename T, typename... Args>
concept invokable_metafunction_for
    = requires { typename T::template invoke<Args...>; };

template <typename T, typename List, template <typename...> typename ListT>
concept unary_metafunction_for_list
    = is_template_of<List, ListT> and[]<typename... Us>(ListT<Us...>)
{
    return (invokable_metafunction_for<T, Us> and ...);
}
(List{});

// a type which accepts a single template parameter and yields a bool. useful
// to pass as predicates to tmp algorithms
template <typename T, typename... Args>
concept predicate_metafunction_for = (requires {
    invokable_metafunction_for<T, Args>;
    { T::template invoke<Args>::value } -> std::convertible_to<bool>;
} and ...);

// for convenience if the predicate arguments are already in a tmcpp::list<>
// TODO: we require a third arg ListT to be a template-template type of the
// given List argument. without ListT, we would require the definition of
// tmcpp::list<> to be in scope, but im trying to keep concepts agnostic of
// library data structures... at least for now. consider if it would make sense
// to include list in concepts here... (i think there is an argument to be
// made)
template <typename T, typename List, template <typename...> typename ListT>
concept predicate_metafunction_for_list
    = is_template_of<List, ListT> and[]<typename... Us>(ListT<Us...>)
{
    return predicate_metafunction_for<T, Us...>;
}
(List{});

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
template <template <typename...> typename T, typename Arg1, typename Arg2>
concept comparator_metafunction = requires {
    { T<Arg1, Arg2>::value } -> std::convertible_to<bool>;
};

template <typename T, typename Arg1, typename Arg2>
concept comparator_metafunction_for = (requires {
    invokable_metafunction_for<T, Arg1, Arg2>;
    { T::template invoke<Arg1, Arg2>::value } -> std::convertible_to<bool>;
});

// TODO: i want to write a 'concept comparator_metafunction_for', but im not
// exactly sure how to write a concept for a metafunction that takes 2 args,
// for each pair of args in a list. we should write one later for consistency

};  // namespace concepts

};  // namespace tmcpp
