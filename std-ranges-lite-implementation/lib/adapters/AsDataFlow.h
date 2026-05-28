#pragma once
#include "BaseAdapter.h"
#include <iterator>

template <typename Input>
class AsDataFlowAdapter {
  Input& range_;
public:
  using value_type = decltype(*(std::begin(range_)));
  AsDataFlowAdapter() = delete;
  AsDataFlowAdapter(Input& r) : range_(r) {};

// NOLINTNEXTLINE
  auto begin() {
    return std::begin(range_);
  }
// NOLINTNEXTLINE
  auto end() {
    return std::end(range_);
  }
// NOLINTNEXTLINE
  auto begin() const {
    return std::begin(range_);
  }
// NOLINTNEXTLINE
  auto end() const {
    return std::end(range_);
  }
};

template <typename Range>
inline auto AsDataFlow(Range& range) {
  return AsDataFlowAdapter<Range>(range);
}