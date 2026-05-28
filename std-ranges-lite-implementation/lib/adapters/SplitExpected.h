#pragma once
#include <vector>
#include <memory>
#include "Filter.h"

template <typename T>
class SharedVectorRange {
  std::shared_ptr<std::vector<T>> data_;
public:
  explicit SharedVectorRange(std::shared_ptr<std::vector<T>> data) : data_(std::move(data)) {}
// NOLINTNEXTLINE
  auto begin() const {return data_->begin();}
// NOLINTNEXTLINE
  auto end() const {return data_->end();}
};

class SplitExpectedAdapter {
public:
  template <typename Range>
  auto Apply(Range&& range) const {
    using ExpectedType = std::remove_cvref_t<decltype(*std::begin(range))>;
    using ValueType = typename ExpectedType::value_type;
    using ErrorType = typename ExpectedType::error_type;
    auto values = std::make_shared<std::vector<ValueType>>();
    auto errors = std::make_shared<std::vector<ErrorType>>();
    for (auto&& item : range) {
      if (item.has_value()) {
        values->push_back(item.value());
      } else {
        errors->push_back(item.error());
      }
    }
    return std::make_pair(SharedVectorRange(errors), SharedVectorRange(values));
  }
};

inline auto SplitExpected() {
  return SplitExpectedAdapter{};
}