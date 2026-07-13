#pragma once

namespace tmcpp
{

// perhaps a better name for this would be overloaded_lambda, since it
// multilambda sounds like there will be multiple lambdas invoked, which is not
// the case; there is one lambda that gets invoked based on the args provided
// to operator()
template <typename... Lambdas> struct multilambda : Lambdas...
{
    using Lambdas::operator()...;
};
// deduction guide to avoid specifying a ctor which might copy the lambdas
template <typename... Lambdas>
multilambda(Lambdas...) -> multilambda<Lambdas...>;

};  // namespace tmcpp
