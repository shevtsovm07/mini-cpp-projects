#pragma once
#include "BaseAdapter.h"
#include <unordered_map>
#include <vector>

template <typename BaseInitializer, typename Aggregator, typename KeyExtractor>
class AggregateByKeyAdapter {
  BaseInitializer base_initializer_;
  Aggregator aggregator_;
  KeyExtractor key_extractor_;

public:
  AggregateByKeyAdapter() = delete;
  AggregateByKeyAdapter(BaseInitializer base_initializer, Aggregator aggregator, KeyExtractor key_extractor) :
  base_initializer_(std::move(base_initializer)), aggregator_(std::move(aggregator)), key_extractor_(std::move(key_extractor)) {};

  template <typename Range>
  auto Apply(Range&& range) {
    using KeyType = std::remove_cvref_t<decltype(key_extractor_(*std::begin(range)))>;
    using ValueType = std::remove_cvref_t<BaseInitializer>;
    std::unordered_map<KeyType, size_t> index_map;
    std::vector<std::pair<KeyType, ValueType>> result;
    for (auto&& item : range) {
      auto key = key_extractor_(item);
      auto key_iterator = index_map.find(key);
      if (key_iterator == index_map.end()) {
        index_map[key] = result.size();
        result.emplace_back(key, base_initializer_);
      } 
      aggregator_(item, result[index_map[key]].second);
    }
    return result;
  }
};


template <typename BaseInitializer, typename Aggregator, typename KeyExtractor>
inline auto AggregateByKey(BaseInitializer base_initializer, Aggregator aggregator, KeyExtractor key_extractor) {
  return AggregateByKeyAdapter<BaseInitializer, Aggregator, KeyExtractor>{base_initializer, aggregator, key_extractor};
}