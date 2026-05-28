#pragma once
#include "BaseAdapter.h"
#include <iterator>

template <typename Range, typename Predicate>
class FilterView {
private:
  Range range_;
  Predicate pred_;
public:
  FilterView() = delete;
  FilterView(Range r, Predicate p) : range_(std::move(r)), pred_(std::move(p)) {};

  struct Iterator{
    using BaseIterator = decltype(std::begin(std::declval<Range&>()));
    BaseIterator cur_;
    BaseIterator end_;
    Predicate* pred_;

    Iterator() = delete;
    Iterator(BaseIterator current, BaseIterator iend, Predicate* p) : cur_(current), end_(iend), pred_(p) {}; 

    Iterator& operator++() {
      if (cur_ != end_) ++cur_;
      while (cur_ != end_ && !(*pred_)(*cur_)) {
        ++cur_;
      }
      return *this;
    }

    auto operator*() const {
      return *cur_;
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
    auto it = std::begin(range_);
    while (it != std::end(range_) && !pred_(*it)) {
      ++it;
    }
    return Iterator{it, std::end(range_), &pred_};
  }
// NOLINTNEXTLINE
  auto end() {
    return Iterator{std::end(range_), std::end(range_), &pred_};
  }
};


template <typename Predicate>
class FilterAdapter {
  Predicate pred_;
public:
  FilterAdapter(Predicate pred) : pred_(std::move(pred)) {};
  
  template <typename Range>
  auto Apply(Range&& range) {
    using CleanRange = std::remove_cvref_t<Range>;
    return FilterView<CleanRange, Predicate>(std::forward<Range>(range), std::move(pred_));
  }
};

template <typename Predicate>
inline auto Filter(Predicate predicate) {
  return FilterAdapter<Predicate>{std::move(predicate)};
}
