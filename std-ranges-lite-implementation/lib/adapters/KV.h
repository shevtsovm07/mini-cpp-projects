#pragma once
#include <utility>

template <typename Key, typename Value>
struct KV {
  using key_type = Key;
  using value_type = Value;

  Key key_;
  Value value_;

  KV() = delete;
  KV(Key k, Value v) : key_(std::move(k)), value_(std::move(v)) {};
};