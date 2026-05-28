#pragma once
#include <utility>

template <typename Range, typename Adapter>
  requires requires(Range&& rang, Adapter&& adapt) {adapt.Apply(std::forward<Range>(rang));}
decltype(auto) operator|(Range&& range, Adapter&& adapter) {
  return adapter.Apply(std::forward<Range>(range));
}