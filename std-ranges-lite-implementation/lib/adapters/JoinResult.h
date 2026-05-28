#pragma once
#include <optional>

template <typename Base, typename Joined>
struct JoinResult {
	Base base_;
	std::optional<Joined> joined_;

  JoinResult() = delete;
  JoinResult(Base b, std::optional<Joined> j = std::nullopt) : base_(std::move(b)), joined_(std::move(j)) {};

  bool operator==(const JoinResult& other) const {
    return (this->base_ == other.base_ && this->joined_ == other.joined_);
  }
};
