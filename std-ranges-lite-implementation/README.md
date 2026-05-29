# Range Adapters

Lazy, composable range adapters for C++23 with pipe syntax. Adapters chain via `|` and evaluate on demand — no intermediate containers unless explicitly requested.

```cpp
Dir(argv[1], /*recursive=*/false)
    | Filter([](const fs::path& p) { return p.extension() == ".txt"; })
    | OpenFiles()
    | Split("\n ,.;")
    | Transform([](const std::string& token) {
        std::string s = token;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
      })
    | AggregateByKey(
        0uz,
        [](const std::string&, size_t& count) { ++count; },
        [](const std::string& token) { return token; }
      )
    | Transform([](const std::pair<std::string, size_t>& s) {
        return std::format("{} - {}", s.first, s.second);
      })
    | Out(std::cout);
```

## Adapters

| Adapter | Description | Memory |
|---|---|---|
| `Dir` | Iterates filesystem entries, optionally recursive | O(1) |
| `Filter` | Skips elements that don't satisfy a predicate | O(1) |
| `OpenFiles` | Opens an `ifstream` for each path in the range | O(1) |
| `Split` | Tokenizes strings by a set of delimiter characters | O(1) |
| `Transform` | Maps a function over each element | O(1) |
| `AggregateByKey` | Groups by key and folds values | O(n) |
| `Join` | LEFT JOIN of two ranges on a key | O(right) |
| `DropNullopt` | Filters `std::nullopt` from `optional<T>` ranges | O(1) |
| `SplitExpected` | Splits `expected<T,E>` range into two pipelines | O(1) |
| `AsDataFlow` | Wraps any container into a pipeable range | O(1) |
| `AsVector` | Collects the range into `std::vector` | O(n) |
| `Write` | Writes elements to a stream with a separator | O(1) |
| `Out` | Prints elements to an output stream | O(1) |

All adapters satisfy the requirements for range-based `for` loops.

## Design

The `|` operator is defined once in `BaseAdapter.h` via a C++20 concept:

```cpp
template <typename Range, typename Adapter>
  requires requires(Range&& r, Adapter&& a) { a.Apply(std::forward<Range>(r)); }
decltype(auto) operator|(Range&& range, Adapter&& adapter) {
  return adapter.Apply(std::forward<Range>(range));
}
```

Each adapter is a view that stores a reference to its upstream range and an iterator into it. No data is copied at construction; iteration is fully lazy. `AggregateByKey` and `Join` are the only exceptions — they must materialise the right-hand side upfront.

`Join` builds an `unordered_map` from the right range once, then performs O(1) lookups per left element. Result type is `JoinResult<L, R>` which holds a left value and an `optional` right value — absent when no match is found.

`std::ranges` is excluded by design; all iterator plumbing is hand-written.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --verbose
```

Requires: C++23, CMake ≥ 3.20, Google Test

## Tech

C++23 · lazy iterators · concepts · `std::expected` · `std::optional` · template metaprogramming · filesystem · Google Test
