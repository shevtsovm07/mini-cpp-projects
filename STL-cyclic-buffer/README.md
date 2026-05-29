# circular_buffer

A fixed-capacity circular buffer implemented as a fully compliant STL container. Satisfies `Container`, `ReversibleContainer`, `SequenceContainer`, and `AllocatorAwareContainer` named requirements — verified at compile time via C++20 concepts.

```cpp
circular_buffer<int> cb(5);

for (int i = 0; i < 20; ++i)
    cb.push_back(i);

// buffer holds the last 5 elements: 15 16 17 18 19
for (auto x : cb)
    std::cout << x << " ";
```

When the buffer is full, `push_back` and `push_front` overwrite the oldest element rather than throwing. Size stays fixed at capacity; no reallocation ever occurs.

## Interface

```cpp
template<typename T, bool Extendable = false, typename Allocator = std::allocator<T>>
class circular_buffer;
```

| Method | Description |
|---|---|
| `push_back(value)` / `push_front(value)` | Add element; overwrites oldest if full |
| `pop_back()` / `pop_front()` | Remove element; throws `std::out_of_range` if empty |
| `front()` / `back()` | Access first/last element |
| `at(index)` | Bounds-checked random access |
| `operator[](index)` | Unchecked random access |
| `insert(pos, ...)` | Insert one value, N copies, iterator range, or `initializer_list` |
| `erase(pos)` / `erase(pos1, pos2)` | Remove element or range |
| `assign(...)` | Replace contents from count+value, iterator range, or `initializer_list` |
| `resize(n)` | Shrink or grow to n elements (n ≤ capacity) |
| `clear()` | Destroy all elements, keep capacity |
| `swap(other)` | O(1) swap |
| `size()` / `empty()` / `max_size()` | Size queries |
| `get_allocator()` | Returns the allocator |

## Iterators

The buffer provides a single `base_iterator<IsConst, IsReverse>` template that covers all four iterator categories:

| Type | Alias |
|---|---|
| `base_iterator<false, false>` | `iterator` |
| `base_iterator<true, false>` | `const_iterator` |
| `base_iterator<false, true>` | `reverse_iterator` |
| `base_iterator<true, true>` | `const_reverse_iterator` |

All four satisfy `LegacyRandomAccessIterator`. Arithmetic (`+`, `-`, `+=`, `-=`), comparison (`<`, `>`, `<=`, `>=`, `==`, `!=`), and `operator[]` are fully implemented. Converting from `iterator` to `const_iterator` is allowed; the reverse is rejected at compile time via `static_assert`.

Iterator arithmetic accounts for the circular layout: `operator*` dereferences at `data_[(head_ + pos) % capacity]`, so logical indices map correctly regardless of where `head_` sits in the underlying array.

## Named requirements compliance

Verified at compile time in `named_requirements_ut.cpp` using C++20 concept definitions:

```cpp
static_assert(Container<circular_buffer<int>>);
static_assert(AllocatorAwareContainer<circular_buffer<int>>);
static_assert(SequenceContainer<circular_buffer<int>>);
static_assert(ReversibleContainer<circular_buffer<int>>);
```

## Implementation notes

- **Storage**: raw memory allocated via `std::allocator_traits`; objects are constructed and destroyed in place with `construct`/`destroy`, never default-initialized
- **Layout**: a single flat array with `head_` index; logical index `i` maps to physical index `(head_ + i) % capacity_`
- **Copy/move**: copy constructor linearises the buffer (resets `head_` to 0); move constructor transfers ownership in O(1); move with a different allocator falls back to element-wise move
- **Copy-and-swap**: copy assignment uses copy-and-swap idiom for strong exception safety

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --verbose
```

Requires: C++23, CMake ≥ 3.14, Google Test + GMock

## Tech

C++23 · STL named requirements · `std::allocator_traits` · random access iterators · concepts · copy-and-swap · Google Test
