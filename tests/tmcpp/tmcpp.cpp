#include "tmcpp/tmcpp.hpp"

#include <gtest/gtest.h>

#include <type_traits>

template <typename A, typename B> using Equivalent = std::is_same<A, B>;

TEST(mputils, bind_front)
{
    // using Types = std::tuple<float, double, bool, int, unsigned>;
    using EquivalentToInt = tmcpp::bind_front<Equivalent, int>;

    static_assert(tmcpp::invoke<EquivalentToInt, int>::value);
    static_assert(not tmcpp::invoke<EquivalentToInt, float>::value);
}

TEST(mputils, find_type_if)
{
    using Types = tmcpp::typelist<float, bool, double, bool, int,
                                          unsigned, bool>;
    using EquivalentToInt = tmcpp::bind_front<Equivalent, int>;
    using EquivalentToBool = tmcpp::bind_front<Equivalent, bool>;
    using EquivalentToLong = tmcpp::bind_front<Equivalent, long>;

    using FoundInt
        = tmcpp::find_type_if<EquivalentToInt::template fn, Types>;
    static_assert(std::is_same_v<FoundInt, int>);

    using FoundBool
        = tmcpp::find_type_if<EquivalentToBool::template fn, Types>;
    static_assert(std::is_same_v<FoundBool, bool>);

    using FoundLong
        = tmcpp::find_type_if<EquivalentToLong::template fn, Types>;
    static_assert(std::is_same_v<FoundLong, void>);
}

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
        = tmcpp::typelist<LabeledType<"one"_L>, LabeledType<"five"_L>,
                                  LabeledType<"target"_L>,
                                  LabeledType<"peanut"_L>>;
    using List2T = tmcpp::typelist<
        LabeledType<"one"_L>, LabeledType<"five"_L>, LabeledType<"target"_L>,
        LabeledType<"peanut"_L>, LabeledType<"target"_L>>;

    using Found1T = tmcpp::find_type_if<
        HasLabelEqualTo<target_label>::template fn, List1T>;
    using Found2T = tmcpp::find_type_if<
        HasLabelEqualTo<target_label>::template fn, List2T>;

    static_assert(std::is_same_v<Found1T, LabeledType<target_label>>);
    // static_assert(std::tuple_size_v<Found1T> == 1);
    static_assert(std::is_same_v<Found2T, LabeledType<target_label>>);
    // TODO: maybe another test in a list without the target; should return
    // void type
}

TEST(mputils, transform)
{
    using Types = tmcpp::typelist<int, double, float, bool>;
    using X = tmcpp::transform<std::add_pointer, Types>;
    static_assert(
        std::is_same_v<
            X, tmcpp::typelist<int *, double *, float *, bool *>>);
}

template <typename A, typename B>
using LabelComparator = std::bool_constant<A::label == B::label>;

TEST(mputils, make_unique_tuple_if)
{
    using namespace tmcpp::literals;

    using List1T = tmcpp::typelist<
        LabeledType<"one"_L>, LabeledType<"five"_L>, LabeledType<"target"_L>,
        LabeledType<"peanut"_L>, LabeledType<"pistaccio"_L>>;
    using List2T = tmcpp::typelist<
        LabeledType<"one"_L>, LabeledType<"five"_L>, LabeledType<"target"_L>,
        LabeledType<"target"_L>, LabeledType<"peanut"_L>,
        LabeledType<"target"_L>, LabeledType<"target"_L>,
        LabeledType<"peanut"_L>>;

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
    == tmcpp::make_unique_typelist_if<LabelComparator,
                                              Typelist>::size>;

// test if we can successfully write an algorithm to check if a typelist
// contains unique types satisfying a predicate using mputils primitives
TEST(mputils, check_if_typelist_has_unique_types_by_label)
{
    using namespace tmcpp::literals;

    using List1T = tmcpp::typelist<
        LabeledType<"one"_L>, LabeledType<"five"_L>, LabeledType<"target"_L>,
        LabeledType<"peanut"_L>, LabeledType<"pistaccio"_L>>;
    using List2T = tmcpp::typelist<
        LabeledType<"one"_L>, LabeledType<"five"_L>, LabeledType<"target"_L>,
        LabeledType<"target"_L>, LabeledType<"peanut"_L>,
        LabeledType<"target"_L>, LabeledType<"target"_L>,
        LabeledType<"peanut"_L>>;

    static_assert(TypesAreUniqueByLabel<List1T>::value);
    static_assert(not TypesAreUniqueByLabel<List2T>::value);
}

TEST(mputils, rename)
{
    using L = tmcpp::typelist<int, bool, double>;
    using T = tmcpp::rename<L, std::tuple>;

    using E = std::tuple<int, bool, double>;

    static_assert(std::is_same_v<T, E>);
}

// TEST(mputils, typelist_type_at)
// {
//     using L = tmcpp::typelist<int, bool, double>;
//     using L0 = std::tuple_element_t<0, L>;
//     using L1 = std::tuple_element_t<1, L>;
//     using L2 = std::tuple_element_t<2, L>;
//
//     static_assert(std::is_same_v<L0, int>);
//     static_assert(std::is_same_v<L1, bool>);
//     static_assert(std::is_same_v<L2, double>);
// }

TEST(mputils, typelist_size)
{
    using L = tmcpp::typelist<int, bool, double>;

    static_assert(L::size == 3);
}

TEST(mputils, typelist_cat)
{
    using L1 = tmcpp::typelist<int, bool, double>;
    using L2 = tmcpp::typelist<float, long>;
    using L3 = tmcpp::typelist<unsigned, void>;

    using L = tmcpp::typelist_cat<L1, L2, L3>;
    using Expected = tmcpp::typelist<int, bool, double, float, long,
                                             unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(mputils, typelist_cat_with_empty_list)
{
    using L1 = tmcpp::typelist<int, bool, double>;
    using L2 = tmcpp::typelist<>;
    using L3 = tmcpp::typelist<unsigned, void>;

    using L = tmcpp::typelist_cat<L1, L2, L3>;
    using Expected
        = tmcpp::typelist<int, bool, double, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(mputils, typelist_cat_with_no_lists)
{
    using Actual = tmcpp::typelist_cat<>;
    using Expected = tmcpp::typelist<>;

    static_assert(std::is_same_v<Actual, Expected>);
}

struct is_int_q
{
    template <typename T> using fn = std::is_same<T, int>;
};

TEST(mputils, find_type_if_q)
{
    using L = tmcpp::typelist<bool, double, float, int, long>;

    using Actual = tmcpp::find_type_if_q<is_int_q, L>;

    static_assert(std::is_same_v<Actual, int>);
}

TEST(mputils, typelist_type_at)
{
    using L = tmcpp::typelist<bool, int, double, float>;
    using Actual0 = tmcpp::typelist_type_at<0, L>;
    using Actual1 = tmcpp::typelist_type_at<1, L>;
    using Actual2 = tmcpp::typelist_type_at<2, L>;
    using Actual3 = tmcpp::typelist_type_at<3, L>;

    static_assert(std::is_same_v<Actual0, bool>);
    static_assert(std::is_same_v<Actual1, int>);
    static_assert(std::is_same_v<Actual2, double>);
    static_assert(std::is_same_v<Actual3, float>);
}

// TODO: FOR NEXT TIME: the issue with compiling using gcc might be related to
// the use of 'typename' on the rhs of a 'using' statement. try putting
// typename everywhere we use template aliases
// NOTE: above todo does not help
// TODO: for next time: investigate if the issue is with
// decltype(lambda(return;)). on clang, this evaluates to 'void', which can be
// assigned to a type alias. but see if this is the reason gcc is having
// trouble. test it on compiler explorere
// UPDATE: above todo does not seem to be an issue....
//
// TODO: perhaps we need to put 'typename' before nested template arguments,
// like 'typelist_cat<typename std::conditional_t<
// Predicate<Types>::value, typelist<Types>, typelist < >> ... >' ?
