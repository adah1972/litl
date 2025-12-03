#ifndef TYPELIST_HPP
#define TYPELIST_HPP

#include <cstddef>
#include <type_traits>

namespace litl {  // Lisp Inspired Type List

/* List manipulation basics */

template <typename... Ts>
struct TypeList {};

template <typename... Ts>
struct MakeList {  // (list v ...)
    using type = TypeList<Ts...>;
};

template <typename Head, typename Tail>
struct Cons;  // (cons head tail)

template <typename Head, typename... Tail>
struct Cons<Head, TypeList<Tail...>> {
    using type = TypeList<Head, Tail...>;
};

template <typename Head, typename Tail>
using Cons_t = typename Cons<Head, Tail>::type;

template <typename List>
struct Car;  // (car lst)

template <typename Head, typename... Tail>
struct Car<TypeList<Head, Tail...>> {
    using type = Head;
};

template <typename List>
using Car_t = typename Car<List>::type;

template <typename List>
struct Cdr;  // (cdr lst)

template <typename Head, typename... Tail>
struct Cdr<TypeList<Head, Tail...>> {
    using type = TypeList<Tail...>;
};

template <typename List>
using Cdr_t = typename Cdr<List>::type;

template <typename List, typename T>
struct Append;  // No direct Scheme equivalent

template <typename... Ts, typename T>
struct Append<TypeList<Ts...>, T> {
    using type = TypeList<Ts..., T>;
};

template <typename List, typename T>
using Append_t = typename Append<List, T>::type;

template <typename List, typename T>
using Prepend = Cons<T, List>;  // Just different argument order from Cons

template <typename List, typename T>
using Prepend_t = typename Prepend<List, T>::type;

template <typename List>
using First = Car<List>;  // Same as Car

template <typename List>
using First_t = typename First<List>::type;

template <typename List>
using Rest = Cdr<List>;  // Same as Cdr

template <typename List>
using Rest_t = typename Rest<List>::type;

template <typename List>
struct Last;

template <typename Head>
struct Last<TypeList<Head>> {
    using type = Head;
};

template <typename Head, typename... Tail>
struct Last<TypeList<Head, Tail...>> {
    using type = typename Last<TypeList<Tail...>>::type;
};

template <typename List>
using Last_t = typename Last<List>::type;

template <typename... Lists>
struct Concatenate;  // (append lst ...)

template <typename List1, typename List2, typename... Lists>
struct Concatenate<List1, List2, Lists...> {
    using type =
        typename Concatenate<typename Concatenate<List1, List2>::type,
                             Lists...>::type;
};

template <typename... Ts, typename... Us>
struct Concatenate<TypeList<Ts...>, TypeList<Us...>> {
    using type = TypeList<Ts..., Us...>;
};

template <typename... Ts>
struct Concatenate<TypeList<Ts...>> {
    using type = TypeList<Ts...>;
};

template <typename... Lists>
using Concatenate_t = typename Concatenate<Lists...>::type;

template <typename List, typename T>
struct Contains;

template <typename Head, typename... Tail, typename T>
struct Contains<TypeList<Head, Tail...>, T>
    : std::bool_constant<Contains<TypeList<Tail...>, T>::value> {};

template <typename... Tail, typename T>
struct Contains<TypeList<T, Tail...>, T> : std::true_type {};

template <typename T>
struct Contains<TypeList<>, T> : std::false_type {};

template <typename List, typename T>
inline constexpr bool Contains_v = Contains<List, T>::value;

template <typename List>
struct Size;  // (length lst)

template <typename... Ts>
struct Size<TypeList<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename List>
inline constexpr std::size_t Size_v = Size<List>::value;

template <typename List, std::size_t Pos>
struct ListGet {  // (list-ref lst pos)
    static_assert(Pos < Size_v<List>, "ListGet: index out of bounds");
    using type = typename ListGet<Cdr_t<List>, Pos - 1>::type;
};

template <typename List>
struct ListGet<List, 0> {
    using type = Car_t<List>;
};

template <typename List, std::size_t Pos>
using ListGet_t = typename ListGet<List, Pos>::type;

template <typename List, std::size_t Pos, typename Item>
struct ListSet;  // (list-set lst pos value)

template <typename Head, typename... Tail, std::size_t Pos, typename Item>
struct ListSet<TypeList<Head, Tail...>, Pos, Item> {
    using type =
        Cons_t<Head,
               typename ListSet<TypeList<Tail...>, Pos - 1, Item>::type>;
};

template <typename Head, typename... Tail, typename Item>
struct ListSet<TypeList<Head, Tail...>, 0, Item> {
    using type = TypeList<Item, Tail...>;
};

template <std::size_t Pos, typename Item>
struct ListSet<TypeList<>, Pos, Item> {
    // Always false if matched here
    static_assert(Pos < 0, "ListSet: index out of bounds");
};

template <typename List, std::size_t Pos, typename Item>
using ListSet_t = typename ListSet<List, Pos, Item>::type;

template <typename T>
struct IsList : std::false_type {};  // (list? v)

template <typename... Ts>
struct IsList<TypeList<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool IsList_v = IsList<T>::value;

template <typename List>
struct IsEmpty : std::false_type {};  // (null? v)

template <>
struct IsEmpty<TypeList<>> : std::true_type {};

template <typename List>
inline constexpr bool IsEmpty_v = IsEmpty<List>::value;

template <typename List>
struct IsNotEmpty : std::true_type {};

template <>
struct IsNotEmpty<TypeList<>> : std::false_type {};

template <typename List>
inline constexpr bool IsNotEmpty_v = IsNotEmpty<List>::value;

/* General higher-order "functions" */

template <template <typename...> class Proc, typename List>
struct Apply;  // (apply proc lst), weaker form than in Scheme

template <template <typename...> class Proc, typename... Args>
struct Apply<Proc, TypeList<Args...>> {
    using type = typename Proc<Args...>::type;
};

template <template <typename...> class Proc, typename List>
using Apply_t = typename Apply<Proc, List>::type;

template <template <typename...> class Pred>
struct Not {
    template <typename... Args>
    struct type : std::bool_constant<!Pred<Args...>::value> {};
    // If you need Not, you usually do not want Not_v.
};

template <template <typename> class Pred, typename List>
struct All;

template <template <typename> class Pred, typename Head, typename... Tail>
struct All<Pred, TypeList<Head, Tail...>>
    : std::conditional_t<!Pred<Head>::value, std::false_type,
                         All<Pred, TypeList<Tail...>>> {};

template <template <typename> class Pred>
struct All<Pred, TypeList<>> : std::true_type {};

template <template <typename> class Pred, typename List>
inline constexpr bool All_v = All<Pred, List>::value;

template <template <typename> class Pred, typename List>
struct Any;

template <template <typename> class Pred, typename Head, typename... Tail>
struct Any<Pred, TypeList<Head, Tail...>>
    : std::conditional_t<Pred<Head>::value, std::true_type,
                         Any<Pred, TypeList<Tail...>>> {};

template <template <typename> class Pred>
struct Any<Pred, TypeList<>> : std::false_type {};

template <template <typename> class Pred, typename List>
inline constexpr bool Any_v = Any<Pred, List>::value;

template <template <typename> class Pred, typename List>
struct None;

template <template <typename> class Pred, typename Head, typename... Tail>
struct None<Pred, TypeList<Head, Tail...>>
    : std::conditional_t<Pred<Head>::value, std::false_type,
                         None<Pred, TypeList<Tail...>>> {};

template <template <typename> class Pred>
struct None<Pred, TypeList<>> : std::true_type {};

template <template <typename> class Pred, typename List>
inline constexpr bool None_v = None<Pred, List>::value;

namespace detail {

template <typename T>
struct Add1;

template <typename T, T V>
struct Add1<std::integral_constant<T, V>>
    : std::integral_constant<T, V + 1> {};

template <typename Proc>
struct EvaluateAndAdd1 : Add1<typename Proc::type> {};

} // namespace detail

template <template <typename> class Pred, typename List>
struct Find;  // (list-index pred lst); but size of list if not found

template <template <typename> class Pred, typename Head, typename... Tail>
struct Find<Pred, TypeList<Head, Tail...>>
    : std::conditional_t<
          Pred<Head>::value,
          std::integral_constant<std::size_t, 0>,
          detail::EvaluateAndAdd1<Find<Pred, TypeList<Tail...>>>> {};

template <template <typename> class Pred>
struct Find<Pred, TypeList<>> : std::integral_constant<std::size_t, 0> {};

template <template <typename> class Pred, typename List>
inline constexpr std::size_t Find_v = Find<Pred, List>::value;

template <template <typename> class Pred, typename List>
struct FindLast;

template <template <typename> class Pred, typename Head, typename... Tail>
struct FindLast<Pred, TypeList<Head, Tail...>> {
    static constexpr std::size_t tail_result =
        FindLast<Pred, TypeList<Tail...>>::value;
    static constexpr std::size_t value =
        (Pred<Head>::value && tail_result == sizeof...(Tail))
            ? 0  // Pred is satisfied for Head and not later
            : tail_result + 1;
};

template <template <typename> class Pred>
struct FindLast<Pred, TypeList<>> : std::integral_constant<std::size_t, 0> {
};

template <template <typename> class Pred, typename List>
inline constexpr std::size_t FindLast_v = FindLast<Pred, List>::value;

/* Filter and related functions */

template <template <typename> class Pred, typename List>
struct Filter;  // (filter pred lst)

template <template <typename> class Pred, typename Head, typename... Tail>
struct Filter<Pred, TypeList<Head, Tail...>> {
    using type = typename std::conditional_t<
        Pred<Head>::value,
        Cons<Head, typename Filter<Pred, TypeList<Tail...>>::type>,
        Filter<Pred, TypeList<Tail...>>>::type;
};

template <template <typename> class Pred>
struct Filter<Pred, TypeList<>> {
    using type = TypeList<>;
};

template <template <typename> class Pred, typename List>
using Filter_t = typename Filter<Pred, List>::type;

template <typename List>
struct Unique;  // (delete-duplicates lst)

template <typename Head, typename... Tail>
struct Unique<TypeList<Head, Tail...>> {
    template <typename T>
    struct IsNotHead : std::bool_constant<!std::is_same_v<T, Head>> {};

    using type = Cons_t<
        Head,
        typename Unique<Filter_t<IsNotHead, TypeList<Tail...>>>::type>;
};

template <>
struct Unique<TypeList<>> {
    using type = TypeList<>;
};

template <typename List>
using Unique_t = typename Unique<List>::type;

template <typename... Lists>
struct Union;  // The first list should not have duplicates

template <typename List1, typename List2, typename... Lists>
struct Union<List1, List2, Lists...> {
    using type =
        typename Union<typename Union<List1, List2>::type, Lists...>::type;
};

template <typename List, typename Head, typename... Tail>
struct Union<List, TypeList<Head, Tail...>> {
    using type =
        typename Union<typename std::conditional_t<
                           Contains_v<List, Head>,
                           Concatenate<List>,
                           Append<List, Head>>::type,
                       TypeList<Tail...>>::type;
};

template <typename List>
struct Union<List, TypeList<>> {
    using type = List;
};

template <typename List>
struct Union<List> {
    using type = List;
};

template <typename... Lists>
using Union_t = typename Union<Lists...>::type;

template <typename... Lists>
struct Intersection;  // The first list should not have duplicates

template <typename List1, typename List2, typename... Lists>
struct Intersection<List1, List2, Lists...> {
    using type =
        typename Intersection<typename Intersection<List1, List2>::type,
                              Lists...>::type;
};

template <typename List1, typename List2>
struct Intersection<List1, List2> {
    template <typename T>
    struct IsInList2 : Contains<List2, T> {};

    using type = Filter_t<IsInList2, List1>;
};

template <typename... Lists>
using Intersection_t = typename Intersection<Lists...>::type;

template <typename List1, typename List2>
struct Difference {  // The first list should not have duplicates
    template <typename T>
    struct IsInList2 : Contains<List2, T> {};

    using type = Filter_t<Not<IsInList2>::template type, List1>;
};

template <typename List1, typename List2>
using Difference_t = typename Difference<List1, List2>::type;

/* Fold (reduce) and related functions */

template <template <typename, typename> class Proc, typename Init,
          typename List>
struct FoldLeft;  // (foldl proc init lst)

template <template <typename, typename> class Proc, typename Init,
          typename Head, typename... Tail>
struct FoldLeft<Proc, Init, TypeList<Head, Tail...>> {
    using type = typename FoldLeft<Proc, typename Proc<Init, Head>::type,
                                   TypeList<Tail...>>::type;
};

template <template <typename, typename> class Proc, typename Init>
struct FoldLeft<Proc, Init, TypeList<>> {
    using type = Init;
};

template <template <typename, typename> class Proc, typename Init,
          typename List>
using FoldLeft_t = typename FoldLeft<Proc, Init, List>::type;

template <template <typename, typename> class Proc, typename Init,
          typename List>
struct FoldRight;  // (foldr proc init lst)

template <template <typename, typename> class Proc, typename Init,
          typename Head, typename... Tail>
struct FoldRight<Proc, Init, TypeList<Head, Tail...>> {
    using type = typename Proc<
        typename FoldRight<Proc, Init, TypeList<Tail...>>::type,
        Head>::type;
};

template <template <typename, typename> class Proc, typename Init>
struct FoldRight<Proc, Init, TypeList<>> {
    using type = Init;
};

template <template <typename, typename> class Proc, typename Init,
          typename List>
using FoldRight_t = typename FoldRight<Proc, Init, List>::type;

template <template <typename, typename> class Proc, typename Init,
          typename List>
using Fold = FoldLeft<Proc, Init, List>;

template <template <typename, typename> class Proc, typename Init,
          typename List>
using Fold_t = typename Fold<Proc, Init, List>::type;

template <template <typename, typename> class Proc, typename List>
struct Reduce {
    static_assert(IsNotEmpty_v<List>, "Reduce: list must not be empty");
    using type = typename Fold<Proc, Car_t<List>, Cdr_t<List>>::type;
};

template <template <typename, typename> class Proc, typename List>
using Reduce_t = typename Reduce<Proc, List>::type;

template <typename List>
using Reverse = FoldLeft<Prepend, TypeList<>, List>;

template <typename List>
using Reverse_t = typename Reverse<List>::type;

/* Map function */

template <template <typename...> class Proc, typename... Lists>
struct Map {  // (map proc lst ...)
    struct ProcessNonEmpty {
        // Extract first elements from all lists: (map car lists)
        using Firsts = typename Map<Car, TypeList<Lists...>>::type;

        // Extract rest from all lists: (map cdr lists)
        using Rests = typename Map<Cdr, TypeList<Lists...>>::type;

        // Apply Proc to the first elements - need to unpack Firsts
        using FirstResult = Apply_t<Proc, Firsts>;

        // Recursively process - need to unpack Rests
        template <typename RList>
        struct MapList;

        template <typename... Rs>
        struct MapList<TypeList<Rs...>> {
            using type = typename Map<Proc, Rs...>::type;
        };

        using RestResult = typename MapList<Rests>::type;

        using type = Cons_t<FirstResult, RestResult>;
    };

    static_assert((All_v<IsEmpty, TypeList<Lists...>> ||
                   None_v<IsEmpty, TypeList<Lists...>>),
                  "Map: all lists must have the same size");

    using type =
        typename std::conditional_t<Any<IsEmpty, TypeList<Lists...>>::value,
                                    MakeList<>, ProcessNonEmpty>::type;
};

// Base case: a single list
template <template <typename> class Proc, typename... Ts>
struct Map<Proc, TypeList<Ts...>> {
    using type = TypeList<typename Proc<Ts>::type...>;
};

template <template <typename...> class Proc, typename... Lists>
using Map_t = typename Map<Proc, Lists...>::type;

template <typename... Lists>
using Zip = Map<MakeList, Lists...>;

template <typename... Lists>
using Zip_t = typename Zip<Lists...>::type;

} // namespace litl

#endif // TYPELIST_HPP
