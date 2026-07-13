#include "tmcpp/tmcpp.hpp"
#include "tmcpp/algorithm.hpp"

#include <gtest/gtest.h>

#include <type_traits>

TEST(list, at)
{
    using L = tmcpp::list<bool, int, double, float>;
    using Actual0 = L::at<0>;
    using Actual1 = L::at<1>;
    using Actual2 = L::at<2>;
    using Actual3 = L::at<3>;
    // should not compile:
    // using Actual4 = tmcpp::at<4, L>;

    static_assert(std::is_same_v<Actual0, bool>);
    static_assert(std::is_same_v<Actual1, int>);
    static_assert(std::is_same_v<Actual2, double>);
    static_assert(std::is_same_v<Actual3, float>);
}

template <typename List>
consteval auto
get_first()
{
    static_assert(not List::is_empty);

    // NOTE: notice we need the 'template' keyword when using 'at<>'... thats
    // ugly
    using first = List::template at<0>;
    return first{};
}

TEST(list, at_within_template_function)
{
    using L = tmcpp::list<bool, int, double, float>;

    using expected = bool;
    using actual = decltype(get_first<L>());

    static_assert(std::is_same_v<actual, expected>);
}

template <typename List>
using get_first_alias = decltype([](){
    static_assert(not List::is_empty);

    using first = List::template at<0>;
    return first{};
}());

TEST(list, at_within_template_type_alias)
{
    using L = tmcpp::list<bool, int, double, float>;

    using expected = bool;
    using actual = get_first_alias<L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(list, size)
{
    using L = tmcpp::list<int, bool, double>;

    static_assert(L::size == 3);
}

TEST(algorithm, concatenate)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<float, long>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected
        = tmcpp::list<int, bool, double, float, long, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(algorithm, concatenate_with_empty_list)
{
    using L1 = tmcpp::list<int, bool, double>;
    using L2 = tmcpp::list<>;
    using L3 = tmcpp::list<unsigned, void>;

    using L = tmcpp::concatenate<L1, L2, L3>;
    using Expected = tmcpp::list<int, bool, double, unsigned, void>;

    static_assert(std::is_same_v<L, Expected>);
}

TEST(algorithm, concatenate_with_no_lists)
{
    using Actual = tmcpp::concatenate<>;
    using Expected = tmcpp::list<>;

    static_assert(std::is_same_v<Actual, Expected>);
}

TEST(algorithm, front_or_non_empty)
{
    using L = tmcpp::list<double, float, int>;

    using expected = double;
    using actual = tmcpp::front_or<L, unsigned>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, front_or_empty)
{
    using L = tmcpp::list<>;

    using expected = unsigned;
    using actual = tmcpp::front_or<L, unsigned>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, front_or_empty_with_void_default)
{
    using L = tmcpp::list<>;

    using expected = void;
    using actual = tmcpp::front_or<L, void>;

    static_assert(std::is_same_v<actual, expected>);
}

template <typename A, typename B> using Equivalent = std::is_same<A, B>;

template <typename T> struct type_equal_to
{
    template <typename U> using invoke = std::is_same<T, U>;
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

TEST(algorithm, find_if_simple)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = bool;
    using actual = tmcpp::find_if<type_equal_to<bool>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, find_if_none)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::not_found;
    using actual = tmcpp::find_if<type_equal_to<unsigned>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

// TODO: add more tests for *_index algorithms
TEST(algorithm, filter_index_simple)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::list<std::integral_constant<std::size_t, 1>,
                                 std::integral_constant<std::size_t, 3>>;
    using actual = tmcpp::filter_index<type_equal_to<bool>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, filter_index_returns_empty_list_if_no_types_satisfy_predicate)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::list<>;
    using actual = tmcpp::filter_index<type_equal_to<unsigned>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, find_index_if_simple)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = std::integral_constant<std::size_t, 2>;
    using actual = tmcpp::find_index_if<type_equal_to<double>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

TEST(algorithm, find_index_if_not_found)
{
    using L = tmcpp::list<int, bool, double, bool, float>;

    using expected = tmcpp::not_found;
    using actual = tmcpp::find_index_if<type_equal_to<unsigned>, L>;

    static_assert(std::is_same_v<actual, expected>);
}

struct add_pointer
{
    template <typename T> using invoke = T *;
};

TEST(algorithm, transform)
{
    using L = tmcpp::list<int, double, float, bool>;

    using expected = tmcpp::list<int *, double *, float *, bool *>;
    using actual = tmcpp::transform<add_pointer, L>;

    static_assert(std::is_same_v<actual, expected>);
}

// removing bind_front<> for now
#if 0
struct are_equal
{
    template <typename T, typename U> using invoke = std::is_same<T, U>;
};

TEST(mputils, bind_front)
{
    // using Types = std::tuple<float, double, bool, int, unsigned>;
    using is_equal_to_int = tmcpp::bind_front<are_equal, int>;

    static_assert(tmcpp::invoke<is_equal_to_int, int>::value);
    static_assert(not tmcpp::invoke<is_equal_to_int, float>::value);
}
#endif

#if 0



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
