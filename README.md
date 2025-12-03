# litl

**L**isp **I**nspired **T**ype **L**ist – A C++17 compile-time
metaprogramming library for type manipulation.

## Overview

litl is a header-only template metaprogramming library that brings
Lisp-style list operations to C++ type systems.  All operations happen
at compile-time, enabling powerful type-level computation with zero
runtime overhead.

## Features

- **Lisp-inspired operations**: Car, Cdr, Cons, Map, Filter, Fold, and
  more
- **Functional programming patterns**: Higher-order metafunctions,
  predicates, composition
- **Set operations**: Union, Intersection, Difference
- **Type queries**: Contains, Find, All, Any, None
- **List manipulation**: Concatenate, Reverse, Unique, Zip, Get/Set by
  index
- **Header-only**: Single file, no dependencies beyond the C++17
  standard library
- **Zero runtime cost**: Pure compile-time metaprogramming

## Quick Example

```cpp
#include "typelist.hpp"

using namespace litl;

using List = TypeList<int, double, char>;

static_assert(Size_v<List> == 3);
static_assert(std::is_same_v<First_t<List>, int>);
static_assert(std::is_same_v<Last_t<List>, char>);

using Rev = Reverse_t<List>;  // TypeList<char, double, int>
```

## Installation

Copy `typelist.hpp` to your project and include it:

```cpp
#include "typelist.hpp"
```

Requires C++17 or later.

## Usage

### Basic List Operations

```cpp
using namespace litl;

using Empty = TypeList<>;
using Numbers = TypeList<int, long, short>;

// Access elements
using Head = Car_t<Numbers>;       // int (Lisp: car)
using Tail = Cdr_t<Numbers>;       // TypeList<long, short> (Lisp: cdr)
using First = First_t<Numbers>;    // int (alias for Car)
using Rest = Rest_t<Numbers>;      // TypeList<long, short> (alias for Cdr)
using LastElem = Last_t<Numbers>;  // short

// Build lists
using WithFloat = Cons_t<float, Numbers>;      // TypeList<float, int, long, short>
using WithDouble = Prepend_t<Numbers, double>; // TypeList<double, int, long, short>
using WithChar = Append_t<Numbers, char>;      // TypeList<int, long, short, char>

// Combine lists
using Combined = Concatenate_t<Numbers, TypeList<float, double>>;
// TypeList<int, long, short, float, double>

// Query lists
static_assert(IsList_v<Numbers>);
static_assert(IsEmpty_v<Empty>);
static_assert(Size_v<Numbers> == 3);
static_assert(Contains_v<Numbers, int>);
```

### Indexed Access

```cpp
using List = TypeList<int, double, char, float>;

using Elem0 = ListGet_t<List, 0>;  // int
using Elem2 = ListGet_t<List, 2>;  // char

using Modified = ListSet_t<List, 1, long>;  // TypeList<int, long, char, float>
```

### Predicates and Queries

```cpp
using Types = TypeList<int*, double, char*, float*>;

static_assert(Any_v<std::is_pointer, Types>);           // true
static_assert(!All_v<std::is_pointer, Types>);          // false
static_assert(Find_v<std::is_pointer, Types> == 0);     // index of first pointer
static_assert(FindLast_v<std::is_pointer, Types> == 3); // index of last pointer
```

### Map, Filter, Reduce

```cpp
// Map: transform each element
using Original = TypeList<int, double, char>;
using Pointers = Map_t<std::add_pointer, Original>;
// TypeList<int*, double*, char*>

// Filter: select elements matching predicate
using Mixed = TypeList<int, double, char, float, long>;
using Integers = Filter_t<std::is_integral, Mixed>;
// TypeList<int, char, long>

// Reduce: reduce list to a single type (implemented with FoldLeft)
template <typename T, typename U>
struct Max {
    using type = std::conditional_t<(sizeof(T) > sizeof(U)), T, U>;
};

using Largest = Reduce_t<Max, TypeList<int, short, long, double>>;
// double (largest type)

// Reverse (implemented with FoldLeft)
using Reversed = Reverse_t<Original>;
// TypeList<char, double, int>
```

### Zip and Multi-List Operations

```cpp
using L1 = TypeList<int, double, char>;
using L2 = TypeList<float, long, short>;

// Zip is implemented in multi-list Map
using Paired = Zip_t<L1, L2>;
// TypeList<TypeList<int, float>,
//          TypeList<double, long>,
//          TypeList<char, short>>

// Another multi-list Map example: apply binary function to
// corresponding elements
template <typename T, typename U>
struct Sum {
    using type = std::integral_constant<int, T::value + U::value>;
};

template <int N>
using Int = std::integral_constant<int, N>;

using Nums1 = TypeList<Int<1>, Int<2>, Int<3>>;
using Nums2 = TypeList<Int<10>, Int<20>, Int<30>>;

using Sums = Map_t<Sum, Nums1, Nums2>;
// TypeList<Int<11>, Int<22>, Int<33>>
```

### Unique (Remove Duplicates)

```cpp
using Duplicates = TypeList<int, double, int, char, double>;
using NoDups = Unique_t<Duplicates>;
// TypeList<int, double, char>
```

### Set Operations

```cpp
using A = TypeList<int, double, char>;
using B = TypeList<char, float, int>;

using UnionAB = Union_t<A, B>;
// TypeList<int, double, char, float>

using Intersect = Intersection_t<A, B>;
// TypeList<int, char>

using Diff = Difference_t<A, B>;
// TypeList<double>
```

### Miscellaneous Higher-Order Functions

```cpp
// Not: negate a predicate
using Types = TypeList<int, void, double>;
static_assert(Any_v<Not<std::is_void>::template type, Types>);  // true (has non-void)

// Apply: apply metafunction to unpacked list
template <typename... Ts>
struct MakeTuple { using type = std::tuple<Ts...>; };

using List = TypeList<int, double, char>;
using Tup = Apply_t<MakeTuple, List>;  // std::tuple<int, double, char>
```

## API Reference

### Core Type

- `TypeList<Ts...>` – The fundamental type list

### List Construction

- `Cons<Head, Tail>` – Prepend element (Lisp: cons)
- `Prepend<List, T>` – Prepend element
- `Append<List, T>` – Append element
- `Concatenate<Lists...>` – Concatenate multiple lists (Lisp: append)

### List Access

- `Car<List>` – First element (Lisp: car)
- `Cdr<List>` – All but first (Lisp: cdr)
- `First<List>` – Alias for Car
- `Rest<List>` – Alias for Cdr
- `Last<List>` – Last element
- `ListGet<List, Pos>` – Element at index (Lisp: list-ref)
- `ListSet<List, Pos, Item>` – Replace element at index (Lisp: list-set)

### List Queries

- `IsList<T>` – Check if type is a TypeList (Lisp: list?)
- `IsEmpty<List>` – Check if list is empty (Lisp: null?)
- `Size<List>` – Number of elements (Lisp: length)
- `Contains<List, T>` – Check if type is in list

### Predicates

- `All<Pred, List>` – All elements satisfy predicate (Lisp: every)
- `Any<Pred, List>` – At least one element satisfies predicate (Lisp:
  any)
- `None<Pred, List>` – No elements satisfy predicate
- `Find<Pred, List>` – Index of first match or size if not found (Lisp:
  list-index)
- `FindLast<Pred, List>` – Index of last match or size if not found

### Transformations

- `Map<Proc, Lists...>` – Apply metafunction to each element (Lisp: map)
- `Filter<Pred, List>` – Select elements matching predicate (Lisp: filter)
- `Unique<List>` – Remove duplicates (Lisp: delete-duplicates)
- `FoldLeft<Proc, Init, List>` – Left fold/reduce (Lisp: foldl)
- `FoldRight<Proc, Init, List>` – Right fold/reduce (Lisp: foldr)
- `Fold<Proc, Init, List>` – Alias for FoldLeft
- `Reduce<Proc, List>` – FoldLeft on `Cdr_t<List>` using `Car_t<List>` as `Init`
- `Reverse<List>` – Reverse list order
- `Zip<Lists...>` – Combine lists element-wise

### Set Operations

- `Union<Lists...>` – Set union
- `Intersection<Lists...>` – Set intersection
- `Difference<List1, List2>` – Set difference

### Miscellaneous Higher-Order

- `Apply<Proc, List>` – Apply metafunction to unpacked list (Lisp: apply)
- `Not<Pred>` – Negate predicate

All metafunctions provide `_t` aliases for `::type` and `_v` aliases for
`::value` where applicable.

## Building and Testing

```bash
# Compile test
c++ -std=c++17 typelist_test.cpp -o typelist_test

# Successful compilation means everything is OK!
```

All tests use `static_assert` for compile-time verification. If
compilation succeeds, all tests pass.

## Licence

See *LICENCE* file.


<!--
vim:autoindent:expandtab:formatoptions=tcqlm:textwidth=72:
-->
