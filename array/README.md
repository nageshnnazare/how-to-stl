# Array — Fixed-Size Stack Wrapper

> An `Array<T, N>` is a compile-time-sized bundle of `N` objects sitting inline
> in the struct — like a raw `T[N]` that knows its own length and speaks STL
> (iterators, `size()`, `at()`). Nothing ever goes to the heap; `N` is fixed
> when you write `Array<int, 5>`.

This is a from-scratch reimplementation of `std::array` built for learning. The
header is [`array.hpp`](array.hpp), runnable examples are in
[`array_example.cpp`](array_example.cpp), and the test suite is in
[`../tests/array_test.cpp`](../tests/array_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | `T data_[N]` inline in the object (stack or embedded) |
| Order | Index order `0 .. N-1` |
| Random access | **Yes**, O(1) by index |
| Grows | **Never** — `N` is a template parameter |
| Heap allocation | **None** |

**Reach for an Array when** the element count is known at compile time and fits
comfortably on the stack — coordinates, small buffers, lookup tables.

**Look elsewhere when** size is unknown or huge (see [`vector`](../vector/README.md))
or you need dynamic growth.

---

## 2. Mental Model

An Array is literally one C array wrapped so it behaves like a tiny container:

```
   Array<int, 5> on the stack
   ┌─────┬─────┬─────┬─────┬─────┐
   │  1  │  2  │  3  │  4  │  5  │   data_[0] .. data_[4]
   └─────┴─────┴─────┴─────┴─────┘
   size() == 5  (constexpr, no runtime counter)
```

No pointer chases, no capacity slack — `sizeof(Array<T,N>) == N * sizeof(T)` (no
extra bookkeeping bytes beyond possible alignment padding).

---

## 3. Internal Representation

```cpp
template<typename T, std::size_t N>
struct Array {
    T data_[N];   // the only storage — N elements inline
};
```

**Invariant:** `size()` always returns `N`. `empty()` is `true` only when `N == 0`.

### vs raw C array `T[N]`

| Feature | `Array<T,N>` | `T[N]` |
|---|---|---|
| Knows length | ✅ `size()` | ❌ decays to pointer |
| STL iterators | ✅ `begin()`/`end()` | manual `&arr[0]` |
| Bounds check | ✅ `at(i)` | none |
| Pass by value | copies all `N` elements | same |

---

## 4. How It Works (Step by Step)

### 4.1 Construction — aggregate initialization

```cpp
Array<int, 5> a = {1, 2, 3, 4, 5};  // full
Array<int, 5> b = {1, 2};            // rest value-initialized (0 for int)
Array<int, 5> c;                     // elements default-constructed
```

Because `Array` is an aggregate (only `data_[N]`), brace init fills slots directly
— no constructors run on the struct itself.

### 4.2 Element access

```
   operator[](i)  →  data_[i]           (unchecked, O(1))
   at(i)          →  throw if i >= N     (checked, O(1))
   front()        →  data_[0]
   back()         →  data_[N-1]         (undefined for N==0)
```

### 4.3 Iteration

Iterators are raw pointers into `data_`:

```
   begin() ──▶ &data_[0]
   end()   ──▶ &data_[N]     (one-past-last, same as vector idiom)
```

Range-based `for`, `std::sort`, `std::accumulate` all work unchanged.

### 4.4 `fill(value)`

```
   for i in 0..N-1:  data_[i] = value;
```

Overwrites every slot — O(N), no allocation.

### 4.5 Multi-dimensional arrays

Nest arrays: `Array<Array<int, 3>, 3>` is a 3×3 matrix — each row is its own
inline `Array<int,3>`.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Array<T, N> a = { ... };` | aggregate brace initialization |
| `Array<T, N> a;` | default-construct each `T` |
| `Array<T, 0> empty;` | zero-size edge case |

### Element access
| Call | Bounds-checked? | Notes |
|---|---|---|
| `operator[](i)` | ❌ | same as raw array |
| `at(i)` | ✅ | throws `std::out_of_range` |
| `front()` / `back()` | ❌ | undefined when `N == 0` |

### Capacity
`size()` → `N` (constexpr), `empty()` → `N == 0`

### Modifiers
`fill(value)` — assign all elements

### Iterators
`begin()` / `end()` — `T*` / `const T*`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `operator[]`, `at`, `front`, `back` | O(1) | |
| `fill` | O(N) | |
| Iteration | O(N) | contiguous scan |
| copy / assign | O(N) | copies all `N` elements |
| Space | O(N) | stack footprint `N * sizeof(T)` |

---

## 7. Usage

```cpp
#include "array/array.hpp"

Array<int, 5> scores = {95, 87, 92, 88, 90};

int first = scores.front();
int safe  = scores.at(2);

scores.fill(0);

for (int s : scores) std::cout << s << ' ';

std::sort(scores.begin(), scores.end());
```

See [`array_example.cpp`](array_example.cpp) for STL algorithms, 2-D matrices,
zero-sized arrays, and aggregate initialization.

---

## 8. Design Decisions & Trade-offs

- **Struct, not class with private buffer.** Matches `std::array` aggregate
  semantics — direct `data_` access is idiomatic in teaching code.
- **No `std::get<I>` / tuple interface.** Omitted for brevity; index access
  suffices.
- **Stack only.** Large `N` risks stack overflow — use vector for big buffers.
- **Zero overhead.** No size field at runtime; compiler often optimizes Array
  uses identically to a raw array.
- **`back()` uses `N-1`.** For `N == 0`, undefined — same as `std::array`.

---

## 9. Common Pitfalls

- **Stack size limits.** `Array<char, 1'000'000>` may crash — heap vector instead.
- **`N` must be known at compile time.** Cannot resize at runtime.
- **`operator[]` has no bounds check.** Use `at()` when index is untrusted.
- **`Array<T,0>` edge case.** `front()`, `back()`, `operator[]` are undefined.
- **Copy assigns all N elements** even if you only use the first few logically.

---

## 10. Comparison with `std::array`

**Same:** fixed `N`, inline storage, aggregate initialization, iterators as
pointers, `at()` bounds checking, zero overhead abstraction.

**Intentionally omitted:** `std::get`, comparison operators, `swap`, reverse
iterators, `fill` in C++20 constexpr extended form beyond our simple loop.

---

## 11. Build & Run

```bash
make run-array      # build + run the examples
make test-array     # build + run the unit tests
make all            # build everything in the repo
```

---

## 12. See Also

- [`vector`](../vector/README.md) — dynamic size, heap growth
- [`string`](../string/README.md) — dynamic char sequence with SSO
- [`deque`](../deque/README.md) — runtime-sized, chunked both-end queue
