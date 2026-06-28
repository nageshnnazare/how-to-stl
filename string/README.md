# String — Dynamic Character Sequence with SSO

> A `String` is a growable, null-terminated sequence of characters — like
> `std::string`, but built from scratch so you can see every allocation decision.
> Short strings (≤ 15 characters) live in an inline buffer inside the object
> itself (Small String Optimization); longer strings spill to the heap. You call
> `append` or `+=`; the class tracks size, capacity, and the trailing `'\0'`.

This is a from-scratch reimplementation of `std::string` built for learning. The
header is [`string.hpp`](string.hpp), runnable examples are in
[`string_example.cpp`](string_example.cpp), and the test suite is in
[`../tests/string_test.cpp`](../tests/string_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Storage | Inline SSO buffer (≤ 15 chars) or heap `char[]` |
| Order | Character sequence left-to-right |
| Random access | **Yes**, O(1) by index |
| Grows | Automatically via `reserve` / `append` |
| Object size | ~40 bytes on 64-bit (`data_`, `size_`, `capacity_`, `sso_buffer_[16]`) |

**Reach for a String when** you need a mutable text buffer with rich operations
(append, insert, find, compare) and C-string compatibility via `c_str()`.

**Look elsewhere when** you only need a non-owning view (C++17 `string_view`) or
a fixed-size buffer on the stack (see [`array`](../array/README.md)).

---

## 2. Mental Model

A String is a pointer plus two counters, with a hidden inline buffer for the
common short-string case:

```
   SSO mode (size_ = 5)                    Heap mode (size_ = 30)
   ┌────────────────────────┐             ┌────────────────────────┐
   │ data_  ●──┐            │             │ data_  ●───────────────┼──▶ heap
   │ size_  = 5             │             │ size_  = 30            │    "Long...\0"
   │ capacity_ = 15         │             │ capacity_ = 30         │
   │ sso_buffer_            │             │ sso_buffer_ (unused)   │
   │   [H][e][l][l][o][\0]◀─┘             └────────────────────────┘
   └────────────────────────┘
```

- `data_` always points at the active buffer — either `sso_buffer_` or heap.
- `size_` is the character count; `data_[size_] == '\0'` is always maintained.
- `is_sso()` is simply `data_ == sso_buffer_` — no separate mode flag.

---

## 3. Internal Representation

```cpp
static constexpr size_type SSO_CAPACITY = 15;

char*       data_;                    // active buffer (SSO or heap)
size_type   size_;                    // char count (excludes '\0')
size_type   capacity_;                // max chars before regrow
char        sso_buffer_[SSO_CAPACITY + 1];  // inline storage + '\0' slot
```

**Invariants:** `0 <= size_ <= capacity_`; `data_[size_] == '\0'`; when in SSO
mode, `data_ == sso_buffer_` and `capacity_ == SSO_CAPACITY`.

### SSO vs heap — the two storage modes

| Mode | Condition | `data_` points to | Heap allocated? |
|---|---|---|---|
| SSO | `size_ <= 15` (and never promoted) | `sso_buffer_` inside object | No |
| Heap | `size_ > 15` or after `reserve(n)` with `n > 15` | `new char[capacity_+1]` | Yes |

---

## 4. How It Works (Step by Step)

### 4.1 Construction from a C-string

```
   String("Hello"):
       len = 5 <= 15  →  stay in SSO
       data_ = sso_buffer_;  strcpy into inline buffer

   String("...30 chars..."):
       len = 30 > 15  →  data_ = new char[31];  strcpy to heap
```

### 4.2 SSO → heap transition (`reserve` / `append`)

When `size_ + growth` would exceed `SSO_CAPACITY`, `ensure_capacity` calls
`reserve`, which allocates heap memory and copies existing characters:

```
   before:  data_ ─▶ sso_buffer_  "Hello\0"        (cap 15, SSO)

   append 20 more chars (total 25 > 15):
       new_data ─▶ [Hello + 20 new chars\0]         (heap, cap ≥ 25)
       delete[] old heap only if was heap; SSO buffer never deleted
       data_ = new_data;  capacity_ updated
```

### 4.3 Append (`append` / `push_back` / `operator+=`)

```
   append " World" to "Hello":
       ensure_capacity(5 + 6)   — no-op if SSO has room
       memcpy(data_ + 5, " World", 6)
       size_ = 11;  data_[11] = '\0'
```

Amortized O(1) when geometric growth is used; this implementation grows to the
exact requested capacity in `reserve` (simpler, but may realloc more often than
libstdc++'s geometric policy).

### 4.4 Insert — memmove opens a gap

```
   insert "XX" at index 1 of "ABCDE":
              [A][B][C][D][E][\0]
               └── memmove tail right by 2 ──▶
              [A][ ][ ][B][C][D][E][\0]
                  └── memcpy "XX" ──▶
              [A][X][X][B][C][D][E][\0]
```

`memmove` (not `memcpy`) because source and destination overlap.

### 4.5 Move semantics — two paths

```
   Move from SSO source:  strcpy inline (can't steal buffer inside other)
   Move from heap source: steal data_ pointer O(1); reset other to empty SSO
```

### 4.6 `swap` — four SSO/heap combinations

Both SSO → strcpy exchange; both heap → swap pointers; mixed → copy SSO side
to temp, steal heap pointer, strcpy temp into the SSO object.

### 4.7 `shrink_to_fit` — heap → SSO when possible

If `size_ <= 15` on a heap string, copy into `sso_buffer_`, `delete[]` heap,
repoint `data_` at `sso_buffer_`.

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `String()` | empty SSO string |
| `String(const char*)` | copy C-string |
| `String(n, c)` | `n` copies of character `c` |
| `String(other, pos, len)` | substring |
| copy / move ctor | deep copy / steal-or-copy |

### Element access
| Call | Bounds-checked? | Notes |
|---|---|---|
| `operator[](i)` | ❌ | fastest; UB if out of range |
| `at(i)` | ✅ | throws `std::out_of_range` |
| `front()` / `back()` | ❌ | first / last character |
| `c_str()` / `data()` | — | null-terminated `const char*` |

### Capacity
`empty()`, `size()`, `length()`, `capacity()`, `reserve(n)`, `shrink_to_fit()`

### Modifiers
`clear()`, `append()`, `operator+=`, `push_back`, `pop_back`, `insert`, `erase`,
`replace`, `resize`, `swap`

### String operations
`find`, `rfind`, `substr`, `compare`

### Non-member
`+` concatenation, `==`, `!=`, `<`, `<=`, `>`, `>=`, `<<`, `>>`, `swap`

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `operator[]`, `at`, `front`, `back` | O(1) | |
| Construction from C-string | O(n) | `n` = length |
| Copy | O(n) | deep copy all chars |
| Move (heap) | O(1) | pointer steal |
| Move (SSO) | O(n) | small `n` ≤ 15 |
| `append` / `push_back` | O(n) worst | may realloc + copy |
| `insert` / `erase` / `replace` | O(n) | tail shift |
| `find` (substring) | O(n·m) | naive `strstr`-based |
| `swap` (both heap) | O(1) | pointer swap |
| `clear` | O(1) | just reset size + `'\0'` |

---

## 7. Usage

```cpp
#include "string/string.hpp"

String s = "Hello";
s += ", World!";           // append (may stay SSO or go heap)
s.push_back('?');

char c = s[0];             // fast, unchecked
char safe = s.at(1);       // checked

s.insert(5, ",");          // "Hello, World!?"
s.replace(0, 5, "Hi");     // "Hi, World!?"

auto pos = s.find("World");
String sub = s.substr(0, 2);

for (char ch : s) std::cout << ch;
```

See [`string_example.cpp`](string_example.cpp) for SSO transitions, move
semantics, search/replace, and more.

---

## 8. Design Decisions & Trade-offs

- **SSO threshold of 15.** Matches a common libstdc++ choice on 64-bit; trades
  ~16 extra bytes per object for zero heap traffic on short strings.
- **Pointer-as-mode-flag.** `data_ == sso_buffer_` means SSO — no extra bool.
- **Exact growth in `reserve`.** Simpler than geometric doubling; easier to
  reason about, but more reallocations on repeated small appends.
- **`strcpy`/`memcpy`/`memmove`.** Pedagogically clear C-style memory ops; a
  production string might use `char_traits` and allocators.
- **Move from SSO copies.** The inline buffer cannot be stolen (it lives inside
  the source object), so SSO move is a small memcpy, not a pointer steal.

---

## 9. Common Pitfalls

- **Assuming move empties SSO sources meaningfully.** After moving from an SSO
  string, the source is still valid with the same characters (copied, not stolen).
- **`data()` returns `const char*` in our API** — use non-const `operator[]` to
  mutate individual characters.
- **Invalidating nothing on `append` to SSO** — but `reserve` that leaves SSO
  allocates new heap memory; any raw pointer into `sso_buffer_` would dangle
  (don't hold raw pointers into string internals).
- **`operator[]` has no bounds check.** Use `at()` on untrusted indices.
- **Forgetting the null terminator.** `size_` excludes `'\0'`; always use
  `c_str()` for C APIs, not `&s[0]` on an empty string before C++11 guarantees.

---

## 10. Comparison with `std::string`

**Same:** SSO idea, dynamic growth, null termination, rich modifier/search API,
move/copy semantics, iterator as `char*`.

**Intentionally omitted for clarity:** custom allocators, `string_view` interop,
locale-aware operations, `std::pmr::string`, COW (deprecated in libstdc++ anyway).

**Differs:** our SSO move copies; growth policy is exact-size `reserve`; object
is slightly larger (explicit 16-byte SSO buffer always present).

---

## 11. Build & Run

```bash
make run-string     # build + run the examples
make test-string    # build + run the unit tests
make all            # build everything in the repo
```

---

## 12. See Also

- [`vector`](../vector/README.md) — contiguous dynamic array (similar growth ideas)
- [`array`](../array/README.md) — fixed-size stack buffer when length is known at compile time
- [`deque`](../deque/README.md) — double-ended queue
- [`list`](../list/README.md) — linked structure, poor cache locality vs contiguous strings
