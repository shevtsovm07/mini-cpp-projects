#pragma once
#include "BaseAdapter.h"
#include <filesystem>
#include <fstream>

template <typename Range>
class OpenFilesView {
  Range range_;
public:
  OpenFilesView() = delete;
  OpenFilesView(Range r) : range_(std::move(r)) {};

  struct Iterator {
    using BaseIter = decltype(std::begin(std::declval<Range&>()));
    BaseIter cur_;
    BaseIter end_;
    std::shared_ptr<std::ifstream> current_file_;

    Iterator() = delete;
    Iterator(BaseIter current, BaseIter finish) : cur_(current), end_(finish) {
      if (cur_ != end_) {
        current_file_ = std::make_shared<std::ifstream>((*cur_).string());
      }
    };

    auto& operator*() {
      return *current_file_;
    }

    Iterator& operator++() {
      ++cur_;
      if (cur_ != end_) {
        current_file_ = std::make_shared<std::ifstream>((*cur_).string());
      }
      return *this;
    }
    bool operator!=(const Iterator& other) const { return cur_ != other.cur_; }
    bool operator==(const Iterator& other) const { return cur_ == other.cur_; }
  };
// NOLINTNEXTLINE
  auto begin() {
    return Iterator{std::begin(range_), std::end(range_)};
  }
// NOLINTNEXTLINE
  auto end() {
    return Iterator{std::end(range_), std::end(range_)};
  }
};

class OpenFilesAdapter {
public:
  template <typename Range>
  auto Apply(Range&& range) {
    using CleanRange = std::remove_cvref_t<Range>;
    return OpenFilesView<CleanRange>(std::forward<Range>(range));
  }
};

inline auto OpenFiles() {
  return OpenFilesAdapter{};
}