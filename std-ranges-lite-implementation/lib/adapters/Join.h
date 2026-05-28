#pragma once
#include "BaseAdapter.h"
#include "KV.h"
#include "JoinResult.h"
#include <unordered_map>

template <typename LeftRange, typename RightRange, typename LeftKeyExtractor, typename RightKeyExtractor, typename RightValueGetter, typename LeftValueGetter>
class JoinView {
  LeftRange left_range_;

  using RightValue = std::remove_cvref_t<decltype(*(std::begin(std::declval<RightRange&>())))>;
  using RightKey = std::remove_cvref_t<decltype(std::declval<RightKeyExtractor&>()(std::declval<RightValue&>()))>;
  using MappedRightValue = std::remove_cvref_t<decltype(std::declval<RightValueGetter>()(std::declval<RightValue&>()))>;
  using RightMap = std::unordered_map<RightKey, MappedRightValue>;

  RightMap map_;
  LeftKeyExtractor left_key_;
  RightKeyExtractor right_key_;
  RightValueGetter right_value_;
  LeftValueGetter left_value_;

public:
  JoinView() = delete;
  JoinView(LeftRange l, RightRange r, LeftKeyExtractor left_key_extr, RightKeyExtractor right_key_extr, RightValueGetter right_value_getter, LeftValueGetter left_value_getter) :
  left_range_(std::move(l)), left_key_(std::move(left_key_extr)), right_key_(std::move(right_key_extr)), right_value_(std::move(right_value_getter)), left_value_(std::move(left_value_getter)) {
    for (auto&& item : r) {
      map_.emplace(right_key_(item), right_value_(item));
    }
  }

  struct Iterator {

    using BaseIter = decltype(std::begin(std::declval<LeftRange&>()));
    using LeftValue = std::remove_cvref_t<decltype(*std::declval<BaseIter>())>;
    using MappedLeftValue  = std::remove_cvref_t<decltype(std::declval<LeftValueGetter>()(std::declval<LeftValue&>()))>;

    RightMap* right_map_ptr_;
    LeftKeyExtractor* left_key_ptr_;
    LeftValueGetter* left_value_ptr_;
    RightValueGetter* right_value_ptr_;
    BaseIter current_pos_;
    BaseIter end_pos_;

    Iterator() = delete;
    Iterator(RightMap* right_map_pointer, LeftKeyExtractor* left_key_pointer, LeftValueGetter* left_value_pointer, RightValueGetter* right_value_pointer,
      BaseIter start, BaseIter finish) : right_map_ptr_(right_map_pointer), left_key_ptr_(left_key_pointer), left_value_ptr_(left_value_pointer), right_value_ptr_(right_value_pointer),
      current_pos_(start), end_pos_(finish) {};

    auto operator*() const {
      auto iterator = right_map_ptr_->find((*left_key_ptr_)(*current_pos_));
      if (iterator != right_map_ptr_->end()) {
        return JoinResult<MappedLeftValue, MappedRightValue>{(*left_value_ptr_)(*current_pos_), iterator->second};
      } else {
        return JoinResult<MappedLeftValue, MappedRightValue>{(*left_value_ptr_)(*current_pos_), std::nullopt};
      }
    }
    
    Iterator& operator++() {
      ++current_pos_;
      return *this;
    }

    bool operator!=(const Iterator& other) const {return current_pos_ != other.current_pos_;}
    bool operator==(const Iterator& other) const {return current_pos_ == other.current_pos_;}
  };
// NOLINTNEXTLINE
  auto begin() {return Iterator{&map_, &left_key_, &left_value_, &right_value_, std::begin(left_range_), std::end(left_range_)};}
// NOLINTNEXTLINE
  auto end() {return Iterator{&map_, &left_key_, &left_value_, &right_value_, std::end(left_range_), std::end(left_range_)};}
};

template <typename RightRange, typename LeftKeyExtractor, typename RightKeyExtractor, typename LeftValueGetter, typename RightValueGetter>
class JoinAdapter {
  RightRange right_range_;
  LeftKeyExtractor left_key_;
  RightKeyExtractor right_key_;
  LeftValueGetter left_value_;
  RightValueGetter right_value_;

public:
  JoinAdapter(RightRange r, LeftKeyExtractor lk, RightKeyExtractor rk, LeftValueGetter lv, RightValueGetter rv)
    : right_range_(std::move(r)), left_key_(std::move(lk)), right_key_(std::move(rk)), left_value_(std::move(lv)), right_value_(std::move(rv)) {}

  template <typename LeftRange>
  auto Apply(LeftRange&& left) {
    using CleanLeft = std::remove_cvref_t<LeftRange>;
    return JoinView<CleanLeft, RightRange, LeftKeyExtractor, RightKeyExtractor, RightValueGetter, LeftValueGetter>(std::forward<LeftRange>(left), right_range_, left_key_, right_key_, right_value_, left_value_);
  }
};

template <typename RightRange>
inline auto Join(RightRange&& right_range) {
  auto key = [](const auto& kv){return kv.key_;};
  auto value = [](const auto& kv){return kv.value_;};
  return JoinAdapter{std::forward<RightRange>(right_range), key, key, value, value};
}

template <typename RightRange, typename LeftKeyExtractor, typename RightKeyExtractor>
inline auto Join(RightRange&& right_range, LeftKeyExtractor left_key_ext, RightKeyExtractor right_key_ext) {
  auto same = [](const auto& value){return value;};
  return JoinAdapter{std::forward<RightRange>(right_range), left_key_ext, right_key_ext, same, same};
}

