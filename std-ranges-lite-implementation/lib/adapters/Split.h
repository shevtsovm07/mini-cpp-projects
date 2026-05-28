#pragma once
#include <string>

template <typename Range>
class SplitView {
  Range range_;
  std::string delimeters_;

public:
  SplitView() = delete;
  SplitView(Range r, std::string dels) : range_(std::move(r)), delimeters_(std::move(dels)) {};

  struct Iterator {
    using BaseIter = decltype(std::begin(std::declval<Range&>()));
    BaseIter cur_;
    BaseIter end_;
    const std::string* delimeters_;
    std::string cur_token_;

    Iterator(BaseIter start, BaseIter finish, const std::string* dels) : cur_(start), end_(finish), delimeters_(dels) {
      FindNext();
    };

    void FindNext() {
      cur_token_.clear();
      while (cur_ != end_) {
        using ElementType = std::remove_cvref_t<decltype(*cur_)>;
        if constexpr (std::is_same_v<ElementType, char>) {
          char ch = *cur_;
          ++cur_;
          if (delimeters_->find(ch) != std::string::npos) {
            if (cur_token_.empty()) continue;
            return;
          }
          cur_token_ += ch;
        } else {
          char ch;
          while ((*cur_).get(ch)) {
            if (delimeters_->find(ch) != std::string::npos) {
              return;
            }
            cur_token_ += ch;
          }
          ++cur_;
          if (!cur_token_.empty()) return;
        }
      }
    }


    auto operator*() const { return cur_token_; }

    Iterator& operator++() {
      FindNext();
      return *this;
    }

    bool operator !=(const Iterator& other) const {
      return !cur_token_.empty() || cur_ != other.cur_;
    }

  };
// NOLINTNEXTLINE
  auto begin() {return Iterator{std::begin(range_), std::end(range_), &delimeters_};}
// NOLINTNEXTLINE
  auto end() {return Iterator{std::end(range_), std::end(range_), &delimeters_};}
};



class SplitAdapter {
  std::string delimeters_;
public:
  SplitAdapter() = delete;
  SplitAdapter(std::string delims) : delimeters_(std::move(delims)) {};
  template <typename Range>
  auto Apply(Range&& range) {
    using CleanRange = std::remove_cvref_t<Range>;
    return SplitView<CleanRange>(std::forward<Range>(range), delimeters_);
  }
};


inline auto Split(const std::string& delims) {
  return SplitAdapter{delims};
}
