# int2025_t

A 2024-bit signed integer type implemented from scratch in C. No standard big-integer libraries — just a 254-byte array and bitwise arithmetic.

## What it is

`int2025_t` is a struct holding 254 bytes of raw data. Internally it uses two's complement representation, the same as native integer types on any modern CPU. This means negative numbers, overflow, and sign extension all behave exactly as they do for `int32_t` or `int64_t`.

```c
int2025_t a = from_int(1000000);
int2025_t b = from_string("99999999999999999999999999999999999999999");
int2025_t c = a * b;
std::cout << c; // prints the full result
```

## Operations

| Operation | Description |
|---|---|
| `from_int(int32_t)` | Construct from a native integer |
| `from_string(const char*)` | Construct from a decimal string |
| `+`, `-`, `*`, `/` | Arithmetic operators |
| `==`, `!=` | Equality comparison |
| `<<` | Stream output in decimal |

## Implementation

**Storage**: 254 bytes, little-endian. Byte 0 holds bits 0–7, byte 253 holds bits 2016–2023. The sign bit is bit 2023 (MSB of the last byte).

**Negation**: two's complement — flip all bits, add 1. This is how `from_int` handles negative inputs and how subtraction is implemented.

**Multiplication**: long multiplication on individual bytes, accumulating carry across the result array.

**Division**: binary long division using bit-level shift and subtract.

**Output**: repeated division by 10 to extract decimal digits, then printed in reverse.

## Constraints

- `sizeof(int2025_t) <= 254` — enforced by `static_assert` at compile time
- No dynamic allocation — the entire value lives on the stack
- No `<big_integer>` or similar libraries

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Requires: C17, CMake ≥ 3.14

## Tech

C · bitwise arithmetic · two's complement · manual carry propagation · binary long division · Google Test
