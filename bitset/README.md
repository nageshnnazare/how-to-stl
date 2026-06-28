# Bitset — Fixed-Size Packed Bit Array

> A `Bitset<N>` is a compile-time-fixed array of N bits packed into machine
> words. Setting, clearing, testing, or flipping bit `i` is a single shift and
> mask on one `unsigned long` — not a byte per flag. Bitwise `&`, `|`, `^`, and
> `~` run word-at-a-time, which is how permission masks, bloom filters, and
> dense set operations are implemented in practice.

This is a from-scratch reimplementation of `std::bitset` built for learning. The
header is [`bitset.hpp`](bitset.hpp), runnable examples are in
[`bitset_example.cpp`](bitset_example.cpp), and the test suite is in
[`../tests/bitset_test.cpp`](../tests/bitset_test.cpp).

---

## 1. What It Is

| Property | Value |
|---|---|
| Size | `N` bits, fixed at compile time |
| Storage | `ceil(N / word_bits)` × `unsigned long` |
| Single-bit ops | O(1) — one word index + one bitmask |
| Bulk bitwise | O(W) where W = number of words |
| Heap | None — `words_` lives inline in the object |

**Reach for a Bitset when** you need a compact boolean array of known size,
feature flags, permission bits, adjacency rows, or set operations on a small universe.

**Look elsewhere when** size is runtime (`vector<bool>`), the set is sparse and
large (hash set), or you need mutable references to individual bits (`vector<bool>`
proxy references).

---

## 2. Mental Model

Logical bit indices map into a word array:

```
   Bitset<10>  (BITS_PER_WORD = 64, NUM_WORDS = 1)

   words_[0]:
   ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬─────────────────────────────┐
   │b9│b8│b7│b6│b5│b4│b3│b2│b1│b0│  unused bits 10..63         │
   └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴─────────────────────────────┘
    9  8  7  6  5  4  3  2  1  0

   bit i  →  word = i / 64 ,  offset = i % 64 ,  mask = 1UL << offset
```

For `N > 64`, bits continue in `words_[1]`, `words_[2]`, …:

```
   Bitset<100>:  words_[0] = bits 0..63 ,  words_[1] = bits 64..99 (+ padding)
```

---

## 3. Internal Representation

```cpp
template<std::size_t N>
class Bitset {
    static constexpr size_t BITS_PER_WORD = sizeof(unsigned long) * 8;
    static constexpr size_t NUM_WORDS = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;
    unsigned long words_[NUM_WORDS];
};
```

**Invariant:** Only the low `N` bits across `words_[0..NUM_WORDS-1]` are
logical bitset bits. `operator~` masks the high garbage bits in the last partial
word so they stay zero.

### Indexing formula

| Quantity | Formula |
|---|---|
| Word index | `pos / BITS_PER_WORD` |
| Bit offset | `pos % BITS_PER_WORD` |
| Mask | `1UL << bit_offset` |

---

## 4. How It Works (Step by Step)

### 4.1 Default construction

```
   Bitset<N> bs;

   for each word: words_[i] = 0    // all N bits clear
```

### 4.2 `set(pos, value)` — single-bit write

```
   word = pos / BITS_PER_WORD
   bit  = pos % BITS_PER_WORD

   if value:  words_[word] |=  (1UL << bit)    // set to 1
   else:       words_[word] &= ~(1UL << bit)    // set to 0

   pos >= N → no-op (silent)
```

### 4.3 `test(pos)` — single-bit read

```
   return (words_[word] & (1UL << bit)) != 0

   pos >= N → false
```

### 4.4 `flip(pos)`

```
   words_[word] ^= (1UL << bit)    // toggle 0↔1
```

### 4.5 `count()` — population count

```
   for each word w:
       while w:  count += w & 1;  w >>= 1
```

Walks every set bit in every word (teaching implementation; production code
often uses hardware popcount).

### 4.6 Word-level `&`, `|`, `^`

```
   result.words_[i] = words_[i] OP other.words_[i]   for i in 0..NUM_WORDS-1
```

One machine instruction per word for AND/OR/XOR on typical CPUs.

### 4.7 `operator~` and the last-word mask

When `N % 64 != 0`, high bits in the last word are not part of the bitset:

```
   ~all bits in words_
   then:  last_word &= (1UL << (N % 64)) - 1   // keep only valid low bits
```

---

## 5. API Reference

### Construction
| Call | Effect |
|---|---|
| `Bitset<N>()` | All bits 0 |
| `Bitset<N>(unsigned long val)` | Low bits of `val` in `words_[0]` |

### Single-bit modifiers
| Call | Effect |
|---|---|
| `set(pos)` | Set bit to 1 |
| `set(pos, false)` | Clear bit |
| `reset(pos)` | Clear bit |
| `reset()` | Clear all N bits |
| `flip(pos)` | Toggle bit |

### Observers
| Call | Effect |
|---|---|
| `test(pos)` | Safe test; false if `pos >= N` |
| `operator[](pos)` | Same as `test` (no mutable proxy) |
| `count()` | Number of 1-bits |
| `size()` | `N` (constexpr) |
| `all()`, `any()`, `none()` | Query helpers via `count()` |

### Bitwise (non-member syntax via member ops)
| Operator | Effect |
|---|---|
| `a & b` | Intersection |
| `a \| b` | Union |
| `a ^ b` | Symmetric difference |
| `~a` | Complement within N bits |

All mutators return `Bitset&` for chaining: `bs.set(0).set(3).flip(1)`.

---

## 6. Complexity Summary

| Operation | Complexity | Note |
|---|---|---|
| `set` / `reset` / `flip` / `test` | O(1) | One word |
| `reset()` (all) | O(W) | W = NUM_WORDS |
| `count`, `all`, `any`, `none` | O(W × word_bits) worst | This implementation |
| `&`, `\|`, `^`, `~` | O(W) | Word loop |
| Space | O(W) words | W = ⌈N / 64⌉ typically |

---

## 7. Usage

```cpp
#include "bitset/bitset.hpp"

Bitset<8> flags;
flags.set(0).set(7);           // bits 0 and 7 on

if (flags.test(0)) { /* ... */ }

Bitset<8> a(0b11110000);
Bitset<8> b(0b10101010);
auto both = a & b;

std::size_t ones = flags.count();
bool every = flags.all();
```

See [`bitset_example.cpp`](bitset_example.cpp) for construction, manipulation,
bitwise logic, queries, permissions, masking, set algebra, 64-bit layouts, and
a bloom-filter sketch.

---

## 8. Design Decisions & Trade-offs

- **`unsigned long` words** — matches typical `std::bitset` word type on Unix;
  size is platform-defined (often 64 bits on LP64).
- **Silent no-op on out-of-range `set`/`flip`** — matches common std behavior
  for mutators; `test` returns `false`.
- **No mutable `operator[]`** — returns `bool` by value, not a bit proxy like
  `std::bitset::reference` (simpler teaching code).
- **Shift-loop `count()`** — clear and portable; not using `popcnt` intrinsics.
- **Fixed `N`** — enables stack storage and compile-time size in the type.

---

## 9. Common Pitfalls

- **Unused bits in the last word.** Never assume `words_[last]` uses all 64 bits
  when `N % 64 != 0`; `~` relies on masking.
- **`operator[]` is read-only here.** You cannot assign through `bs[i] = true`;
  use `set(i)`.
- **Platform word size.** `BITS_PER_WORD` follows `unsigned long`; portable code
  should not hard-code 64 without checking.
- **`all()` on empty `Bitset<0>`** — edge case: vacuously true in math; verify
  behavior if you use `N == 0`.

---

## 10. Comparison with `std::bitset`

**Same:** fixed N, word packing, bit index arithmetic, bitwise ops, `count`,
`all`/`any`/`none`.

**Intentionally omitted:** `to_string`, `to_ulong`, `to_ullong`, stream I/O,
`find`, `flip()` without position, and proxy references for `operator[]`.

---

## 11. Build & Run

```bash
make run-bitset     # build + run the examples
make test-bitset    # build + run the unit tests
make all            # build everything in the repo
```

From the repo root:

```bash
g++ -std=c++14 -Wall -Wextra -Wpedantic -I. bitset/bitset_example.cpp -o /tmp/x_bitset
/tmp/x_bitset
```

---

## 12. See Also

- [`array`](../array/README.md) — fixed-size array of full objects, not bits
- [`vector`](../vector/README.md) — dynamic size; `vector<bool>` is also bit-packed
- [`set`](../set/README.md) — sparse ordered keys when the universe is huge
