#include "tmcpp/tmcpp.hpp"
#include "tmcpp/algorithm.hpp"

#include <gtest/gtest.h>

#include <type_traits>

TEST(list, at)
{
    using L = tmcpp::list<bool, int, double, float>;
    using Actual0 = tmcpp::at<0, L>;
    using Actual1 = tmcpp::at<1, L>;
    using Actual2 = tmcpp::at<2, L>;
    using Actual3 = tmcpp::at<3, L>;

    static_assert(std::is_same_v<Actual0, bool>);
    static_assert(std::is_same_v<Actual1, int>);
    static_assert(std::is_same_v<Actual2, double>);
    static_assert(std::is_same_v<Actual3, float>);
}

TEST(list, size)
{
    using L = tmcpp::list<int, bool, double>;

    static_assert(L::size == 3);
}

TEST(list, concatenate)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<float, long>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected
        = tmcpp::list<int, bool, double, float, long, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(list, concatenate_with_empty_list)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected = tmcpp::list<int, bool, double, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(list, concatenate_with_no_lists)
{
    using Actual = tmcpp::concatenate<>;
    using Expected = tmcpp::list<>;

    static_assert(std::is_same_v<Actual, Expected>);
}

template <typename A, typename B> using Equivalent = std::is_same<A, B>;

template <typename T> struct type_equal_to
{
    template <typename U> static constexpr bool value = std::is_same_v<T, U>;
};

TEST(algorithm, filter_simple)
{
    using L = tmcpp::list<int, double, bool, float, bool, bool>;

    using expected = tmcpp::list<bool, bool, bool>;
    using actual = tmcpp::filter<type_equal_to<bool>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, filter_empty)
{
    using L = tmcpp::list<>;

    using expected = tmcpp::list<>;
    using actual = tmcpp::filter<type_equal_to<int>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, find_type_if_simple)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = bool;
    using actual = tmcpp::find_type_if<type_equal_to<bool>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

#if 0

TEST(mputils, bind_front)
{
    // using Types = std::tuple<float, double, bool, int, unsigned>;
    using EquivalentToInt = tmcpp::bind_front<Equivalent, int>;

    static_assert(tmcpp::invoke<EquivalentToInt, int>::value);
    static_assert(not tmcpp::invoke<EquivalentToInt, float>::value);
}

TEST(mputils, find_type_if)
{
    using Types = tmcpp::list<float, bool, double, bool, int, unsigned, bool>;

    using FoundInt = tmcpp::find_type_if<type_equal_to<int>, Types>;
    static_assert(std::is_same_v<FoundInt, int>);

    using FoundBool = tmcpp::find_type_if<type_equal_to<bool>, Types>;
    static_assert(std::is_same_v<FoundBool, bool>);

    using FoundLong = tmcpp::find_type_if<type_equal_to<long>, Types>;
    static_assert(std::is_same_v<FoundLong, void>);
}
#endif

#if 0

//
template <tmcpp::ConstexprString Label> struct HasLabelEqualTo
{
    template <typename LabeledT>
    using fn = std::bool_constant<LabeledT::label == Label>;
};

// some arbitrary struct that has a label member
template <tmcpp::ConstexprString Label> struct LabeledType
{
    static constexpr auto label = Label;
};

// this test represents how i intend to use these constructs in tmcpp
TEST(mputils, find_type_if_using_constexpr_string_label)
{
    using namespace tmcpp::literals;

    constexpr auto target_label = "target"_L;
    // using TargetT
    //     = std::integral_constant<decltype(target_label), target_label>;
    // using HasLabelEqualToTarget
    //     = tmcpp::bind_front<HasLabelEqualTo, TargetT>;

    using List1T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>>;
    using List2T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>,
                      LabeledType<"target"_L>>;

    using Found1T
        = tmcpp::find_type_if<HasLabelEqualTo<target_label>::template fn,
                              List1T>;
    using Found2T
        = tmcpp::find_type_if<HasLabelEqualTo<target_label>::template fn,
                              List2T>;

    static_assert(std::is_same_v<Found1T, LabeledType<target_label>>);
    // static_assert(std::tuple_size_v<Found1T> == 1);
    static_assert(std::is_same_v<Found2T, LabeledType<target_label>>);
    // TODO: maybe another test in a list without the target; should return
    // void type
}

TEST(mputils, transform)
{
    using Types = tmcpp::list<int, double, float, bool>;
    using X = tmcpp::transform<std::add_pointer, Types>;
    static_assert(
        std::is_same_v<X, tmcpp::list<int *, double *, float *, bool *>>);
}

template <typename A, typename B>
using LabelComparator = std::bool_constant<A::label == B::label>;

TEST(mputils, make_unique_tuple_if)
{
    using namespace tmcpp::literals;

    using List1T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>,
                      LabeledType<"pistaccio"_L>>;
    using List2T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"target"_L>,
                      LabeledType<"peanut"_L>, LabeledType<"target"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>>;

    using List1UniqueT
        = tmcpp::make_unique_typelist_if<LabelComparator, List1T>;
    using List2UniqueT
        = tmcpp::make_unique_typelist_if<LabelComparator, List2T>;

    // NOTE: currently, we are not checking that the computed 'unique' lists
    // contain the same types. currently, i am not interested in the order of
    // the types, and i think the implementation potentially removes types from
    // the front, so idk how to check for that here... this is good enough for
    // now
    static_assert(List1UniqueT::size == 5);
    static_assert(List2UniqueT::size == 4);
}

template <typename Typelist>
using TypesAreUniqueByLabel = std::bool_constant<
    Typelist::size
    == tmcpp::make_unique_typelist_if<LabelComparator, Typelist>::size>;

// test if we can successfully write an algorithm to check if a typelist
// contains unique types satisfying a predicate using mputils primitives
TEST(mputils, check_if_typelist_has_unique_types_by_label)
{
    using namespace tmcpp::literals;

    using List1T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>,
                      LabeledType<"pistaccio"_L>>;
    using List2T
        = tmcpp::list<LabeledType<"one"_L>, LabeledType<"five"_L>,
                      LabeledType<"target"_L>, LabeledType<"target"_L>,
                      LabeledType<"peanut"_L>, LabeledType<"target"_L>,
                      LabeledType<"target"_L>, LabeledType<"peanut"_L>>;

    static_assert(TypesAreUniqueByLabel<List1T>::value);
    static_assert(not TypesAreUniqueByLabel<List2T>::value);
}

TEST(mputils, rename)
{
    using L = tmcpp::list<int, bool, double>;
    using T = tmcpp::rename<L, std::tuple>;

    using E = std::tuple<int, bool, double>;

    static_assert(std::is_same_v<T, E>);
}




struct is_int_q
{
    template <typename T> using fn = std::is_same<T, int>;
};

TEST(mputils, find_type_if_q)
{
    using L = tmcpp::list<bool, double, float, int, long>;

    using Actual = tmcpp::find_type_if_q<is_int_q, L>;

    static_assert(std::is_same_v<Actual, int>);
}

#endif
