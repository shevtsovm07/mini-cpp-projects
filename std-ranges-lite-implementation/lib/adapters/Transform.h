#pragma once
#include "BaseAdapter.h"
#include <iterator>

template <typename Range, typename Function>
class TransformView {
private:
  Range range_;
  Function func_;

public:
  TransformView() = delete;
  TransformView(Range r, Function f) : range_(std::move(r)), func_(std::move(f)) {};

  struct Iterator {
    using BaseIterator = decltype(std::begin(std::declval<Range&>()));
    BaseIterator cur_;
    Function* func_ptr_;

    Iterator(BaseIterator it, Function* f) : cur_(it), func_ptr_(f) {};

    auto operator*() const {
      return (*func_ptr_)(*cur_);
    }

    Iterator& operator++() {
      ++cur_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return cur_ == other.cur_;
    }

    bool operator!=(const Iterator& other) const {
      return cur_ != other.cur_;
    }
  };
// NOLINTNEXTLINE
  auto begin() {
    return Iterator{std::begin(range_), &func_};
  }
// NOLINTNEXTLINE
  auto end() {
    return Iterator{std::end(range_), &func_};
  }
};

template <typename Func>
class TransformAdapter {
  Func func_;
public:
  TransformAdapter(Func f) : func_(std::move(f)) {};
  
  template <typename Range>
  auto Apply(Range&& range) {
    using CleanRange = std::remove_cvref_t<Range>;
    return TransformView<CleanRange, Func>(std::forward<Range>(range), std::move(func_));
  }
};

template <typename Func>
inline auto Transform(Func f) {
  return TransformAdapter<Func>{std::move(f)};
}