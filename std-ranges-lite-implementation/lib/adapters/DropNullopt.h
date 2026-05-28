#pragma once
#include "BaseAdapter.h"
#include <optional>

template <typename Range>
class DropNulloptView {
  Range range_;

public:
  DropNulloptView() = delete;
  DropNulloptView(Range&& r) : range_(std::move(r)) {};

  class Iterator {
    using BaseIterator = decltype(std::begin(std::declval<Range&>()));
    BaseIterator current_;
    BaseIterator end_;
  public:
    Iterator() = delete;
    Iterator(BaseIterator cur, BaseIterator it_end) : current_(cur), end_(it_end) {};

    auto operator*() {
      return *(*current_);
    }

    Iterator& operator++() {
      if (current_ != end_) current_++;
      while (current_ != end_ && !current_->has_value()) {
        current_++;
      }
      return *this;
    }

    bool operator!=(const Iterator& other) const {return current_ != other.current_;}
    bool operator==(const Iterator& other) const {return current_ == other.current_;}
  };
// NOLINTNEXTLINE
  auto begin() {
    auto it = std::begin(range_);
    while (it != std::end(range_) && !it->has_value()) {
      it++;
    }
    return Iterator{it, std::end(range_)};
  }
// NOLINTNEXTLINE
    auto end() {
    return Iterator{std::end(range_), std::end(range_)};
  }
};

class DropNulloptAdapter {
public:
  template <typename Range>
  auto Apply(Range&& range) {
    using CleanRange = std::remove_cvref_t<Range>;
    return DropNulloptView<CleanRange>(std::forward<Range>(range));
  }
};

inline auto DropNullopt() {
  return DropNulloptAdapter{};
}