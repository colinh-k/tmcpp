#pragma once

namespace tmcpp
{

template <typename... Lambdas> struct multilambda : Lambdas...
{
    using Lambdas::operator()...;
};
// deduction guide to avoid specifying a ctor which might copy the lambdas
template <typename... Lambdas>
multilambda(Lambdas...) -> multilambda<Lambdas...>;

};  // namespace tmcpp
