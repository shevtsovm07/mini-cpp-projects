#pragma once
#include "BaseAdapter.h"
#include <iostream>

class OutAdapter {
  std::ostream& os_;

public:
  OutAdapter() = delete;
  OutAdapter(std::ostream& flow) : os_(flow) {};

  template <typename Range>
  void Apply(Range&& range) {
    for (auto&& item : range) {
      os_ << item << " ";
    }
    os_ << std::endl;
  }
};

inline auto Out(std::ostream& out) {
  return OutAdapter{out};
}
