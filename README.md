# ITMO Labs — Systems Programming

First-year coursework in systems programming at ITMO University. Four projects in C++, each focused on a different low-level topic.

---

### int2025_t

A 2024-bit signed integer type built from scratch in C. Stores values in a 254-byte array using two's complement, with full arithmetic (`+`, `-`, `*`, `/`) and decimal I/O implemented via bitwise operations and manual carry propagation. No big-integer libraries.

---

### circular_buffer

A fixed-capacity circular buffer that fully satisfies the STL `Container`, `ReversibleContainer`, `SequenceContainer`, and `AllocatorAwareContainer` named requirements — verified at compile time via C++20 concepts. Implements a single `base_iterator<IsConst, IsReverse>` template covering all four iterator categories with full random-access support.

---

### HamArc

A command-line archiver that stores files with Hamming(8,4) error-correction codes. Every 4 bits of original data become 8 bits in the archive with interleaved parity; single-bit errors are corrected on extraction, two-bit errors are detected. Supports create, extract, list, append, delete, and merge operations.

---

### Range Adapters

Lazy, composable range adapters with pipe syntax, implemented without `std::ranges`. Thirteen adapters including `Dir`, `Filter`, `OpenFiles`, `Split`, `Transform`, `AggregateByKey`, `Join`, `DropNullopt`, and `SplitExpected` — all lazy except where materialisation is unavoidable.

---

## Build

Each project builds independently with CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --verbose
```
