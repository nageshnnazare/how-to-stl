# Tuple — Fixed-Size Heterogeneous Product Type

> A `Tuple<Types...>` holds a fixed number of values, each with its own type,
> accessed by compile-time index via `get<I>(t)`. Unlike `Pair`, there are no
> names — only positions 0, 1, 2 — but the idea is the same: return multiple
> results from a function without heap allocation or void* gymnastics.

This is a from-scratch reimplementation of `std::tuple` (2- and 3-element
specializations) built for learning. The header is [`tuple.hpp`](tuple.hpp),
runnable examples are in [`tuple_example.cpp`](tuple_example.cpp), and the test
suite is in [`../tests/tuple_test.cpp`](../tests/tuple_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Arity | 2 or 3 elements (this teaching header) |
| Access | `get<I>(t)` and `t.get<I>()` — index must be compile-time constant |
| Storage | Inline members per specialization |
| Type safety | `get<2>` on a 2-tuple is a compile error |
| Object size | Sum of member sizes plus alignment padding |

**Reach for a Tuple when** you need three or more typed slots (or two slots
without `first`/`second` naming), especially as a function return type.

**Look elsewhere when** you always have exactly two named fields (see
[`pair`](../pair/README.md)) or need runtime-varying arity (not in this repo).

---

## 2. Mental Model

### What this file implements (flat specializations)

```
   Tuple<int, double, std::string>
   ┌─────────┬──────────┬─────────────┐
   │ int     │ double   │ std::string │
   │ first_  │ second_  │ third_      │
   └─────────┴──────────┴─────────────┘
     get<0>    get<1>     get<2>
```

`get<I>()` is a compile-time dispatch — no runtime index, no virtual calls.

### How full `std::tuple` nests (conceptual — for learning)

The standard library uses recursive inheritance so one template handles any
arity:

```
   Tuple<T1, T2, T3>                    Tuple<T2, T3>              Tuple<T3>
   ┌──────────────────────────┐         ┌─────────────────┐        ┌──────┐
   │ T1 head_                 │         │ T2 head_        │        │ T3   │
   │ ┌──────────────────────┐ │  extends│ ┌─────────────┐ │extends │head_ │
   │ │ Tuple<T2,T3>  tail   │ │◀────────│ │ Tuple<T3>   │ │◀────── │      │
   │ └──────────────────────┘ │         │ └─────────────┘ │        └──────┘
   └──────────────────────────┘         └─────────────────┘

   get<0>(t)  →  head of outer Tuple
   get<1>(t)  →  get<0>(tail)     ← recurse into base once
   get<2>(t)  →  get<0>(get<0>(tail))  ← recurse twice
```

Our 3-element specialization skips the recursion and maps `I` directly to
`first_` / `second_` / `third_` — same user-visible behavior, easier to read.

---

## 3. Internal Representation

```cpp
// 2-element specialization
template<typename T1, typename T2>
class Tuple<T1, T2> {
    T1 first_;
    T2 second_;
};

// 3-element specialization
template<typename T1, typename T2, typename T3>
class Tuple<T1, T2, T3> {
    T1 first_;
    T2 second_;
    T3 third_;
};
```

**Invariant:** Element `I` is always stored in the member selected by `get<I>()`
at compile time. No element is default-constructed lazily — all members exist for
the lifetime of the Tuple.

### Index → member mapping

| Index `I` | 2-tuple member | 3-tuple member |
|---|---|---|
| 0 | `first_` | `first_` |
| 1 | `second_` | `second_` |
| 2 | — (ill-formed) | `third_` |

---

## 4. How It Works (Step by Step)

### 4.1 Construction

```
   Tuple<int, double, string> t(42, 3.14, "hi");

   (1) default or value-init each member in order
   (2) first_  ← 42
       second_ ← 3.14
       third_  ← "hi" (moved or copied)
```

### 4.2 `get<I>()` — compile-time dispatch

```
   template<size_t I> auto& get() {
       if constexpr (I == 0) return first_;
       else if constexpr (I == 1) return second_;
       ...
   }

   Call site:  get<1>(t)

   Compiler keeps ONLY the I==1 branch → reference to second_
   No runtime if; invalid I → compile error
```

In a recursive `std::tuple`, the same logical steps walk the inheritance chain
`I` times; here we jump straight to the member.

### 4.3 Free function `get<I>(t)`

Forwards to the member template so syntax matches the standard library:

```
   get<2>(t)  →  t.template get<2>()
```

### 4.4 Equality

Element-wise AND across all indices:

```
   t == u  iff  get<0>(t)==get<0>(u) && get<1>(t)==get<1>(u) && ...
```

### 4.5 `make_tuple`

Perfect-forwarding factory — identical role to `make_pair`, but for 2 or 3 args.

---

## 5. API Reference

### Construction (2-tuple)
| Call | Effect |
|---|---|
| `Tuple<T1,T2>()` | Value-init both |
| `Tuple(a, b)` | Copy-construct |
| `Tuple(T1&&, T2&&)` | Move-construct |

### Construction (3-tuple)
| Call | Effect |
|---|---|
| `Tuple<T1,T2,T3>()` | Value-init all three |
| `Tuple(a, b, c)` | Copy-construct |
| `Tuple(T1&&, T2&&, T3&&)` | Move-construct |

### Access
| Call | Returns |
|---|---|
| `t.get<I>()` | Reference to element `I` |
| `get<I>(t)` | Same via free function |

### Comparison
| Operator | Semantics |
|---|---|
| `operator==` | All elements equal |

### Non-member
| Call | Effect |
|---|---|
| `make_tuple(...)` | Deduced construction (2 or 3 args) |

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| Construct / destroy | O(1) per element | Plus each `Ti` cost |
| `get<I>()` | O(1) | Inlined to member address |
| `operator==` | O(1) per element | |
| Copy / move | Depends on element types | |

---

## 7. Usage

```cpp
#include "tuple/tuple.hpp"

Tuple<int, double, std::string> t(42, 3.14, "hello");

int& n = get<0>(t);
n = 100;

auto person = make_tuple(std::string("Alice"), 30);

auto divide = [](int a, int b) -> Tuple<bool, int, std::string> {
    if (b == 0) return {false, 0, "Division by zero"};
    return {true, a / b, "Success"};
};

auto [ok, val, msg] = divide(10, 2);  // structured bindings (C++17)
```

See [`tuple_example.cpp`](tuple_example.cpp) for 2- and 3-tuples, `make_tuple`,
multi-value returns, and equality.

---

## 8. Design Decisions & Trade-offs

- **Explicit 2/3 specializations** instead of full recursive variadic tuple —
  easier to read in one file; mirrors how `get` *behaves* without inheritance noise.
- **`if constexpr` in `get`** — requires C++17 in the header; compiles as an
  extension under `-std=c++14` on Clang/GCC with warnings.
- **No `tuple_cat`, tie, or relational ops** — keeps focus on storage and index access.
- **Flat layout** — same cache locality as recursive `std::tuple` for small arity.

---

## 9. Common Pitfalls

- **Using a runtime variable as index.** `get<i>(t)` requires `i` to be a
  compile-time constant; use `std::variant` or a custom type erasure for dynamic choice.
- **Wrong arity.** `get<2>` on `Tuple<int,int>` does not compile — by design.
- **Large tuples on the stack.** Every element is embedded; huge strings inside
  a tuple inflate every copy.
- **Assuming this header is a full `std::tuple` drop-in.** Arity is limited to 2–3.

---

## 10. Comparison with `std::tuple`

**Same:** index access, `make_tuple`, element-wise `==`, zero-overhead abstraction.

**Intentionally omitted:** arbitrary arity, `tuple_size`, `tuple_element`,
`apply`, `tuple_cat`, comparison beyond `==`, and allocator support.

**Different implementation:** flat members + `if constexpr` dispatch vs recursive
inheritance — observably similar for 2–3 elements.

---

## 11. Build & Run

```bash
make run-tuple     # build + run the examples
make test-tuple    # build + run the unit tests
make all           # build everything in the repo
```

From the repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. tuple/tuple_example.cpp -o /tmp/x_tuple
/tmp/x_tuple
```

---

## 12. See Also

- [`pair`](../pair/README.md) — two named members instead of indices
- [`optional`](../optional/README.md) — zero or one value, not a fixed product
- [`map`](../map/README.md) — uses `pair` for entries; could return `tuple` from APIs
