#pragma once
#include "BaseAdapter.h"
#include <iostream>

class WriteAdapter {
  std::ostream& os_;
  char delimeter_;
public:
  WriteAdapter(std::ostream& flow, char del) : os_(flow), delimeter_(del) {}

  template <typename Range>
  std::ostream& Apply(Range&& range) {
    for (auto&& item : range) {
      os_ << item << delimeter_;
    }
    return os_;
  }
};

inline auto Write(std::ostream& out, char delimeter) {
  return WriteAdapter{out, delimeter};
}
