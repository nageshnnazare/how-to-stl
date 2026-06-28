# Optional — Maybe-Value Container

> An `Optional<T>` either holds a live value of type `T` or holds nothing at all.
> The empty state is explicit and type-safe — no `nullptr`, no `-1` sentinel, no
> guessing whether default-constructed `T` means "missing". You check `has_value()`
> or `if (opt)` before you read; `value_or` gives you a fallback without exceptions.

This is a from-scratch reimplementation of `std::optional` built for learning. The
header is [`optional.hpp`](optional.hpp), runnable examples are in
[`optional_example.cpp`](optional_example.cpp), and the test suite is in
[`../tests/optional_test.cpp`](../tests/optional_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| States | **Engaged** (has value) or **disengaged** (empty) |
| Storage | Inline `alignas(T)` byte buffer + `bool` flag |
| Heap | None — value lives inside the Optional object |
| Empty check | `has_value()`, `explicit operator bool` |
| Safe access | `value()` throws; `value_or(default)` never throws |

**Reach for an Optional when** a function might not produce a result, a config
field might be unset, or you want clear "absent" semantics without exceptions.

**Look elsewhere when** the value is always present (use `T` directly), you need
multiple error kinds (exceptions or a `Result` type), or you need reference
semantics across optional slots.

---

## 2. Mental Model

Two states, one object — the `bool` flag is the source of truth:

```
   DISENGAGED                           ENGAGED
   ┌─────────────────────────┐          ┌─────────────────────────┐
   │ storage_: raw bytes     │          │ storage_: live T        │
   │   (no T constructed)    │          │   (placement-new'd)     │
   ├─────────────────────────┤          ├─────────────────────────┤
   │ has_value_ = false      │          │ has_value_ = true       │
   └─────────────────────────┘          └─────────────────────────┘
   Dereference → UB (or throw via        *opt / opt-> / value() OK
   value())                              after check
```

Engaging and disengaging are manual lifetime events:

```
   empty ──Optional(v)──▶ engaged     engaged ──reset()──▶ empty
              placement new                    explicit ~T()
```

---

## 3. Internal Representation

```cpp
template<typename T>
class Optional {
    alignas(T) unsigned char storage_[sizeof(T)];  // raw buffer
    bool has_value_;                                // engaged flag
};
```

**Invariant:** A `T` object exists in `storage_` if and only if `has_value_`
is `true`. Access through `ptr()` without that guard is undefined behavior.

### Why not `T member_` directly?

| Approach | Empty Optional | Problem |
|---|---|---|
| `T value_; bool flag` | Must default-construct `T` | No "truly empty" for non-defaultable `T` |
| Raw storage + flag | No `T` ctor until engaged | Correct optional semantics |

### The four lifetime primitives

| Step | What | Code in `optional.hpp` |
|---|---|---|
| Raw buffer | `alignas(T) char[sizeof(T)]` | member `storage_` |
| Engage | placement new | `new (storage_) T(...)` |
| Disengage | explicit destroy | `ptr()->~T()` |
| Access | reinterpret | `reinterpret_cast<T*>(storage_)` |

---

## 4. How It Works (Step by Step)

### 4.1 Default construction (disengaged)

```
   Optional<int> o;

   storage_   : uninitialized bytes (not an int yet)
   has_value_ : false
   ~Optional  : no ~T() — nothing to destroy
```

### 4.2 Engaging with a value

```
   Optional<string> o("hello");

   (1) has_value_ = true
   (2) new (storage_) string("hello")   // placement new in raw buffer
```

### 4.3 `reset()` — disengage

```
   ENGAGED:
       ptr()->~T()        // string destructor runs
       has_value_ = false // storage_ is raw again
```

### 4.4 Copy and move

```
   Copy from engaged other:
       new (storage_) T(*other.ptr())

   Move from engaged other:
       new (storage_) T(std::move(*other.ptr()))
       other.reset()     // leave source empty
```

### 4.5 Access paths

| API | Empty behavior | When to use |
|---|---|---|
| `value()` | throws `runtime_error` | You expect a value |
| `value_or(d)` | returns `d` | Fallback / config defaults |
| `*opt`, `opt->` | UB if empty | After `if (opt)` guard |

### 4.6 Destructor

```
   ~Optional():
       if has_value_: ptr()->~T()
```

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Optional<T>()` | Disengaged |
| `Optional<T>(const T&)` | Engaged, copy |
| `Optional<T>(T&&)` | Engaged, move |
| copy / move ctor | Duplicate or steal engaged state |

### Observers
| Call | Effect |
|---|---|
| `has_value()` | `true` if engaged |
| `explicit operator bool` | Same as `has_value()` |
| `value()` | Reference; throws if empty |
| `value_or(default)` | Copy of value or default |

### Modifiers
| Call | Effect |
|---|---|
| `reset()` | `~T()` if engaged, then disengage |
| `operator=` | Destroy current, copy/move other's state |

### Dereference
| Call | Note |
|---|---|
| `*opt`, `opt->` | No check — UB if empty |

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| Construct empty | O(1) | No `T` ctor |
| Construct with value | O(1) or O(n) | Depends on `T` |
| `has_value`, `bool` | O(1) | |
| `value`, `*`, `->` | O(1) | |
| `reset` | O(1) or O(n) | Runs `~T()` |
| Copy / move | Cost of `T` | |

**Space:** `sizeof(T)` (aligned) + `sizeof(bool)` + possible tail padding.

---

## 7. Usage

```cpp
#include "optional/optional.hpp"

Optional<int> divide(int a, int b) {
    if (b == 0) return Optional<int>();
    return Optional<int>(a / b);
}

auto r = divide(10, 2);
if (r) {
    std::cout << *r << '\n';
}

Optional<std::string> name;  // unset
std::string display = name.value_or("Guest");

Optional<int> port(8080);
port.reset();
```

See [`optional_example.cpp`](optional_example.cpp) for empty vs engaged states,
`value_or`, exceptions, reset, complex types with visible ctors/dtors, function
returns, copy/move, and config patterns.

---

## 8. Design Decisions & Trade-offs

- **Aligned raw storage** — correct for any `T`; avoids always-constructed member.
- **`runtime_error` on empty `value()`** — teaching clarity; `std::optional` uses
  `std::bad_optional_access`.
- **Move leaves source empty** — via `other.reset()` after stealing; matches
  standard optional move semantics.
- **No `emplace`, `swap`, or comparisons** — smaller surface for learning.
- **Stack-only** — large `T` makes every Optional bulky; use `Optional<unique_ptr<U>>`
  for indirection when needed.

---

## 9. Common Pitfalls

- **Dereferencing without a check.** `*opt` on an empty optional is UB — same as
  `*nullptr`.
- **Confusing "empty" with "value is zero".** `Optional<int>(0)` is engaged;
  `if (opt)` is true even when `*opt == 0`.
- **Forgetting that `value_or` copies.** For expensive `T`, consider references or
  `optional<const T&>` patterns outside this teaching type.
- **Double destruction.** Never call `~T()` on storage unless engaged; the
  implementation guards every path with `has_value_`.

---

## 10. Comparison with `std::optional`

**Same:** engaged/disengaged states, placement new, `value_or`, bool test, move
empties source.

**Intentionally omitted:** `std::nullopt`, `emplace`, `swap`, relational ops,
`std::bad_optional_access`, and monadic operations (`and_then`, C++23).

---

## 11. Build & Run

```bash
make run-optional     # build + run the examples
make test-optional    # build + run the unit tests
make all              # build everything in the repo
```

From the repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. optional/optional_example.cpp -o /tmp/x_optional
/tmp/x_optional
```

---

## 12. See Also

- [`pair`](../pair/README.md) — always holds two values (no empty state)
- [`tuple`](../tuple/README.md) — fixed product of N values
- [`map`](../map/README.md) — `find` returns iterator, not optional (in this repo)
