#pragma once
#include <vector>
#include <type_traits>
#include "BaseAdapter.h"
class AsVectorAdapter {
public:
  template <typename Range> 
  auto Apply(Range&& range) {
    using value_type = std::remove_cvref_t<decltype(*std::begin(range))>;
    std::vector<value_type> result;
    for (auto&& item : range) {
      result.push_back(item);
    }
    return result;
  }
};

inline auto AsVector() {
  return AsVectorAdapter{};
}