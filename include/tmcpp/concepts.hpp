#pragma once

#include <concepts>
#include <expected>

namespace tmcpp
{

namespace concepts
{

template <template <typename...> typename T, typename S>
concept metafunction_for = requires { typename T<S>::type; };

// a type which accepts a single template parameter and yields a bool. useful
// to pass as predicates to tmp algorithms
template <template <typename...> typename T, typename Arg>
concept predicate_metafunction_for = requires {
    { T<Arg>::value } -> std::convertible_to<bool>;
};

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

// TODO: multilambda should be in its own file
template <typename... Lambdas> struct multilambda : Lambdas...
{
    using Lambdas::operator()...;
};
// deduction guide to avoid specifying a ctor which might copy the lambdas
template <typename... Lambdas>
multilambda(Lambdas...) -> multilambda<Lambdas...>;

// constrains T to be a template type matching TargetT, but T and TargetT may
// have different template arguments
// eg is_template_of<std::tuple<int, float>, std::tuple> is legal
template <typename T, template <typename...> typename TargetT>
concept is_template_of = decltype(multilambda{
    []<typename... U>(const TargetT<U...> &) { return std::true_type{}; },
    [](const auto &) { return std::false_type{}; },
}(std::declval<T>()))::value;

};  // namespace concepts

};  // namespace tmcpp
