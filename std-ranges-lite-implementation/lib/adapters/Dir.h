#pragma once
#include <filesystem>
#include "BaseAdapter.h"

class Dir {
  std::filesystem::path root_;
  bool is_recursive_;

public:
  Dir(std::filesystem::path p, bool r = false) : root_(std::move(p)), is_recursive_(r) {};

  struct Iterator {
    std::filesystem::recursive_directory_iterator cur_;
    bool is_recursive_;

    Iterator() = delete;
    Iterator(std::filesystem::recursive_directory_iterator iter, bool is_rec) : cur_(iter), is_recursive_(is_rec) {};

    Iterator& operator++() {
      if (!is_recursive_) {
        cur_.disable_recursion_pending();
      }
      ++cur_;
      return *this;
    }

    auto operator*() const {return cur_->path();}
    bool operator==(const Iterator& other) const {return cur_ == other.cur_;}
    bool operator!=(const Iterator& other) const {return cur_ != other.cur_;}
  };
// NOLINTNEXTLINE
  auto begin() {return Iterator{std::filesystem::recursive_directory_iterator(root_), is_recursive_}; }
// NOLINTNEXTLINE
  auto end() {return Iterator{std::filesystem::recursive_directory_iterator(), is_recursive_}; }

};