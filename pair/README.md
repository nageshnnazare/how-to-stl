# Pair — Two-Value Heterogeneous Aggregate

> A `Pair<T1, T2>` is the smallest heterogeneous container: two values of
> (possibly different) types, exposed as public members `first` and `second`.
> It is the building block behind every associative-container entry — a Map
> stores `Pair<Key, Value>`, and functions use it to return two related results
> without inventing a one-off struct.

This is a from-scratch reimplementation of `std::pair` built for learning. The
header is [`pair.hpp`](pair.hpp), runnable examples are in
[`pair_example.cpp`](pair_example.cpp), and the test suite is in
[`../tests/pair_test.cpp`](../tests/pair_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Arity | Exactly 2 elements |
| Access | Public members `first`, `second` (also structured bindings) |
| Storage | Inline stack aggregate — no heap |
| Ordering | Lexicographic `operator<` (first, then second) |
| Object size | `sizeof(T1) + padding + sizeof(T2)` — not always a simple sum |

**Reach for a Pair when** you need two related values — map entries, coordinates,
`(success, result)` return tuples, or anything `std::pair` would model.

**Look elsewhere when** you need three or more heterogeneous slots (see
[`tuple`](../tuple/README.md)) or a key-only ordered type (see
[`map`](../map/README.md), which wraps Pair internally).

---

## 2. Mental Model

A Pair is not a tiny vector — it is a named product type. Both members live
contiguously inside one object:

```
   Pair<int, std::string> on the stack
   ┌────────────────────┬──────────────────────────────┐
   │ first  : int = 42  │ second : string = "answer"   │
   └────────────────────┴──────────────────────────────┘
     named access           named access
     p.first                p.second
```

Lexicographic ordering compares the left column first; only on a tie does it
look at the right:

```
   (1, 9)  vs  (2, 0)   →  compare first: 1 < 2  →  (1,9) wins (is smaller)
   (1, 3)  vs  (1, 9)   →  first ties → compare second: 3 < 9
```

---

## 3. Internal Representation

```cpp
template<typename T1, typename T2>
struct Pair {
    T1 first;    // primary element (key, x, status flag, …)
    T2 second;   // secondary element (value, y, payload, …)
};
```

**Invariant:** Both members are always present and default-constructible via
`Pair()`. There is no optional or "empty" state — unlike `Optional`.

### Memory layout and padding

Members are laid out in declaration order. The compiler may insert padding so
`second` satisfies `alignof(T2)`:

```
   Pair<int, char>  (typical 64-bit platform, 8-byte sizeof)
   ┌──────────┬─────┬──────────────────┐
   │ int (4B) │char │ 3B padding       │
   └──────────┴─────┴──────────────────┘
   offset 0           offset 4 (aligned to 4 or 8)
```

`make_pair` is a thin perfect-forwarding factory — it does not change layout,
only deduces `T1`/`T2` at the call site.

---

## 4. How It Works (Step by Step)

### 4.1 Construction

```
   Pair<int, string> p(42, "hi");

   (1) allocate Pair object on stack
   (2) construct first  ← copy/move 42 into first
   (3) construct second ← copy/move "hi" into second
```

Default construction value-initializes both members (`0`, empty string, etc.).

### 4.2 `make_pair` and perfect forwarding

```
   auto p = make_pair(1, std::string("x"));

   std::forward preserves value category:
       literal 1        → copied into int
       std::string("x") → moved if rvalue, copied if lvalue
```

### 4.3 Lexicographic `operator<`

```
   bool a < b:
       if a.first < b.first        return true
       if b.first < a.first        return false   // a is greater
       return a.second < b.second  // first tied
```

This is the same ordering `std::map<Pair<K,V>, …>` would use if you ever keyed
on full pairs.

### 4.4 Structured bindings (C++17)

Because `first` and `second` are public members, decomposition works:

```cpp
Pair<int, std::string> p{42, "answer"};
auto [code, label] = p;   // code == 42, label == "answer"
```

### 4.5 Copy vs move

| Operation | Work performed |
|---|---|
| Copy ctor | Copy-construct both members |
| Move ctor | Move-construct both members (cheap for strings) |
| `operator=` | Member-wise copy/move assign (defaulted) |

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Pair<T1,T2>()` | Value-init both members |
| `Pair(f, s)` | Copy from lvalues |
| `Pair(T1&&, T2&&)` | Move from rvalues |
| copy / move ctor | Member-wise (defaulted) |

### Members
| Member | Type | Role |
|---|---|---|
| `first` | `T1` | Primary element |
| `second` | `T2` | Secondary element |

### Comparisons
| Operator | Semantics |
|---|---|
| `==`, `!=` | Member-wise equality |
| `<` | Lexicographic (first, then second) |

### Non-member
| Call | Effect |
|---|---|
| `make_pair(a, b)` | Construct `Pair` with deduced types |

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| Construct / destroy | O(1) per member | Dominated by `T1`, `T2` costs |
| Access `first`/`second` | O(1) | Direct member |
| `==`, `!=`, `<` | O(1) per member | Assumes element compare is O(1) |
| Copy / move | O(1) or O(n) | Depends on element types (e.g. string) |

---

## 7. Usage

```cpp
#include "pair/pair.hpp"

// Explicit types
Pair<int, std::string> p(42, "answer");

// Factory with deduction
auto coords = make_pair(10.5, 20.3);

// Return multiple values
Pair<bool, int> divide(int a, int b) {
    if (b == 0) return {false, 0};
    return {true, a / b};
}

// Structured bindings (C++17)
auto [ok, quotient] = divide(10, 2);

// Sorting pairs lexicographically
Pair<int,int> a{1, 9}, b{2, 0};
bool a_before_b = a < b;   // true: 1 < 2
```

See [`pair_example.cpp`](pair_example.cpp) for construction, `make_pair`,
coordinates, map-style entries, comparisons, and structured bindings.

---

## 8. Design Decisions & Trade-offs

- **Public members** — matches `std::pair` and enables structured bindings;
  no accessors means no encapsulation (acceptable for a passive aggregate).
- **Lexicographic `<` only** — this header provides `==`, `!=`, and `<`; other
  relational operators can be derived the same way `std::pair` does in `<utility>`.
- **No `pair<T&, U&>`-style reference specializations** — keeps the teaching
  implementation small; the standard library has additional refinements.
- **Stack-only** — both values inline; large `T2` bloats every Pair object.

---

## 9. Common Pitfalls

- **Assuming `sizeof(Pair) == sizeof(T1) + sizeof(T2)`.** Alignment padding
  can add bytes between or after members.
- **Using `operator<` when you only care about `first`.** Map compares keys
  alone; if you sort `Pair<K,V>` vectors, the value breaks ties.
- **Returning pairs of heavy types by value.** Prefer move-friendly types or
  `Pair<T, unique_ptr<U>>` for indirection when payloads are large.
- **Forgetting that `Pair` is always "full".** For optional results use
  [`optional`](../optional/README.md) or `Pair<bool, T>` explicitly.

---

## 10. Comparison with `std::pair`

**Same:** two public members, `make_pair`, lexicographic `<`, aggregate-like use.

**Intentionally omitted for clarity:** `std::tie`, `std::pair` relational ops
beyond `<`, piecewise construction, `swap` specialization, and reference-pair
utilities.

---

## 11. Build & Run

```bash
make run-pair     # build + run the examples
make test-pair    # build + run the unit tests
make all          # build everything in the repo
```

From the repo root without Make:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. pair/pair_example.cpp -o /tmp/x_pair
/tmp/x_pair
```

---

## 12. See Also

- [`tuple`](../tuple/README.md) — three or more heterogeneous elements by index
- [`map`](../map/README.md) — associative container whose entries are pairs
- [`optional`](../optional/README.md) — maybe-present single value
