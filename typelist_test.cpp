// Compile-time test suite for litl type list library
// All tests use static_assert - successful compilation means all tests
// passed

#include "typelist.hpp"
#include <tuple>
#include <type_traits>

// ============================================================================
// Test Helper Types
// ============================================================================

// Convenience alias for integral constants
template <int N>
using Int = std::integral_constant<int, N>;

// Predicate: checks if integral constant is even
template <typename T>
struct IsEven;

template <int N>
struct IsEven<std::integral_constant<int, N>>
    : std::bool_constant<(N % 2) == 0> {};

// Metafunction: returns the "larger" type
template <typename T, typename U>
struct Max {
    using type = std::conditional_t<(sizeof(T) >= sizeof(U)), T, U>;
};

// Metafunction: squares an integral constant
template <typename T>
struct Square;

template <int N>
struct Square<std::integral_constant<int, N>> {
    using type = std::integral_constant<int, N * N>;
};

// Metafunction: sums two integral constants
template <typename T, typename U>
struct Sum;

template <int N, int M>
struct Sum<std::integral_constant<int, N>, std::integral_constant<int, M>> {
    using type = std::integral_constant<int, N + M>;
};

// Metafunction: converts type list to std::tuple
template <typename... Ts>
struct MakeTuple {
    using type = std::tuple<Ts...>;
};

// Tool to quickly check a type inside an editor/IDE
template <typename T>
struct TypeDisplayer;
#define TYPE_DISPLAY(x) TypeDisplayer<decltype(x)> type_test

// ============================================================================
// Test Suite
// ============================================================================

int main()
{
    using namespace litl;

    // Common test data
    using Nil = TypeList<>;
    using NumSeq = TypeList<Int<1>, Int<2>, Int<3>>;

    // ------------------------------------------------------------------------
    // Basic Type Predicates
    // ------------------------------------------------------------------------

    static_assert(IsList_v<Nil>);
    static_assert(IsList_v<NumSeq>);
    static_assert(!IsList_v<int>);

    static_assert(IsEmpty_v<Nil>);
    static_assert(IsEmpty<Nil>{});
    static_assert(Not<IsNotEmpty>::type<Nil>{});
    static_assert(IsNotEmpty_v<NumSeq>);
    static_assert(!IsNotEmpty_v<Nil>);

    // ------------------------------------------------------------------------
    // Basic Accessors
    // ------------------------------------------------------------------------

    static_assert(std::is_same_v<First_t<NumSeq>, Int<1>>);
    static_assert(std::is_same_v<Last_t<NumSeq>, Int<3>>);
    static_assert(std::is_same_v<Last_t<TypeList<Int<42>>>, Int<42>>);
    static_assert(std::is_same_v<Rest_t<NumSeq>, TypeList<Int<2>, Int<3>>>);
    static_assert(Size_v<NumSeq> == 3);

    // ------------------------------------------------------------------------
    // List Construction
    // ------------------------------------------------------------------------

    using T1 = Prepend_t<Nil, Int<0>>;
    static_assert(std::is_same_v<T1, TypeList<Int<0>>>);

    using T2 = Concatenate_t<T1, NumSeq, Nil>;
    static_assert(Size_v<T2> == 4);
    static_assert(std::is_same_v<Rest_t<T2>, NumSeq>);

    using T3 = Append_t<NumSeq, Int<4>>;
    static_assert(
        std::is_same_v<T3, TypeList<Int<1>, Int<2>, Int<3>, Int<4>>>);
    static_assert(std::is_same_v<Last_t<T3>, Int<4>>);

    // Multiple list concatenation
    using MultiConcat = Concatenate_t<TypeList<Int<1>>, TypeList<Int<2>>,
                                      TypeList<Int<3>>, TypeList<Int<4>>>;
    static_assert(std::is_same_v<MultiConcat, T3>);

    // ------------------------------------------------------------------------
    // List Access and Modification
    // ------------------------------------------------------------------------

    static_assert(std::is_same_v<ListGet_t<T2, 0>, Int<0>>);
    static_assert(std::is_same_v<ListGet_t<T2, 1>, Int<1>>);
    static_assert(std::is_same_v<ListGet_t<T2, 3>, Int<3>>);

    using T4 = ListSet_t<T2, 3, Int<4>>;
    static_assert(std::is_same_v<ListGet_t<T4, 3>, Int<4>>);

    // ------------------------------------------------------------------------
    // Predicates and Search
    // ------------------------------------------------------------------------

    static_assert(Contains_v<NumSeq, Int<1>>);
    static_assert(!Contains_v<NumSeq, Int<0>>);

    static_assert(!All_v<IsEven, NumSeq>);  // [1,2,3] not all even
    static_assert(Any_v<IsEven, NumSeq>);   // [1,2,3] has 2
    static_assert(!None_v<IsEven, NumSeq>);

    static_assert(Find_v<IsEven, NumSeq> == 1);  // first even at index 1
    static_assert(Find_v<IsEven, TypeList<Int<1>, Int<3>, Int<5>>> ==
                  3);  // not found

    static_assert(FindLast_v<IsEven, NumSeq> ==
                  1);  // only one even at index 1
    static_assert(FindLast_v<IsEven, TypeList<Int<2>, Int<1>, Int<4>>> ==
                  2);  // last even at index 2
    static_assert(FindLast_v<IsEven, TypeList<Int<1>, Int<3>, Int<5>>> ==
                  3);  // not found

    // ------------------------------------------------------------------------
    // Higher-Order Functions: Map
    // ------------------------------------------------------------------------

    static_assert(std::is_same_v<Map_t<Square, NumSeq>,
                                 TypeList<Int<1>, Int<4>, Int<9>>>);

    using T5 = Map_t<IsEven, NumSeq>;
    static_assert(
        std::is_same_v<T5, TypeList<std::false_type, std::true_type,
                                    std::false_type>>);

    // Multi-argument map (zipWith equivalent)
    using Nums1 = TypeList<Int<10>, Int<20>, Int<30>>;
    using MultiMap = Map_t<Sum, NumSeq, Nums1>;
    static_assert(
        std::is_same_v<MultiMap, TypeList<Int<11>, Int<22>, Int<33>>>);

    // ------------------------------------------------------------------------
    // Higher-Order Functions: Zip and Apply
    // ------------------------------------------------------------------------

    static_assert(
        std::is_same_v<
            Zip_t<NumSeq, TypeList<Int<1>, Int<4>, Int<9>>>,
            TypeList<TypeList<Int<1>, Int<1>>,
                     TypeList<Int<2>, Int<4>>,
                     TypeList<Int<3>, Int<9>>>>);

    static_assert(std::is_same_v<Apply_t<MakeTuple, NumSeq>,
                                 std::tuple<Int<1>, Int<2>, Int<3>>>);
    static_assert(std::is_same_v<Apply_t<MakeTuple, Nil>, std::tuple<>>);

    // ------------------------------------------------------------------------
    // Fold Operations
    // ------------------------------------------------------------------------

    // FoldLeft with Prepend reverses the list
    static_assert(std::is_same_v<FoldLeft_t<Prepend, Nil, NumSeq>,
                                 TypeList<Int<3>, Int<2>, Int<1>>>);

    // FoldRight with Prepend preserves order
    static_assert(
        std::is_same_v<FoldRight_t<Prepend, Nil, NumSeq>, NumSeq>);

    // Fold for summation
    static_assert(std::is_same_v<Fold_t<Sum, Int<0>, NumSeq>, Int<6>>);

    // Reduce can be slightly simpler
    static_assert(std::is_same_v<Reduce_t<Sum, NumSeq>, Int<6>>);

    // Reduce is more natural in the Max example
    using Largest = Reduce_t<Max, TypeList<int, short, long long, char>>;
    static_assert(std::is_same_v<Largest, long long>);

    // Reverse implementation
    static_assert(std::is_same_v<Reverse_t<NumSeq>,
                                 TypeList<Int<3>, Int<2>, Int<1>>>);

    // ------------------------------------------------------------------------
    // Filter and Unique
    // ------------------------------------------------------------------------

    static_assert(
        std::is_same_v<Filter_t<IsEven, NumSeq>, TypeList<Int<2>>>);

    static_assert(
        std::is_same_v<Unique_t<T5>,
                       TypeList<std::false_type, std::true_type>>);

    // ------------------------------------------------------------------------
    // Set Operations
    // ------------------------------------------------------------------------

    static_assert(std::is_same_v<Union_t<NumSeq, T1>,
                                 TypeList<Int<1>, Int<2>, Int<3>, Int<0>>>);

    static_assert(std::is_same_v<Intersection_t<Rest_t<NumSeq>, T2>,
                                 TypeList<Int<2>, Int<3>>>);

    static_assert(
        std::is_same_v<Difference_t<T2, NumSeq>, TypeList<Int<0>>>);

    // ------------------------------------------------------------------------
    // Edge Cases: Empty Lists
    // ------------------------------------------------------------------------

    static_assert(std::is_same_v<Concatenate_t<Nil>, Nil>);
    static_assert(std::is_same_v<Filter_t<IsEven, Nil>, Nil>);
    static_assert(std::is_same_v<Reverse_t<Nil>, Nil>);
    static_assert(std::is_same_v<Unique_t<Nil>, Nil>);
}
