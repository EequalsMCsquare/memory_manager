#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace libmem {
class Batch
{
  std::string_view arena_name_;
  const size_t     id_;

  // ? Are the two attributes below neccessary
  size_t min_chunksz_;
  size_t max_chunksz_;

  // TODO: shm_handle

  // TODO: static_bin
};

}