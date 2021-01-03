#pragma once

#include "base_bin.hpp"
#include <map>
#include <string_view>

namespace libmem {
class instant_bin : base_bin
{
private:
  std::string_view                  arena_name_;
  std::map<int, libshm::shm_handle> segments_;

public:
  explicit instant_bin(std::atomic_size_t& segment_counter,
                       std::string_view    arena_name);
};
}