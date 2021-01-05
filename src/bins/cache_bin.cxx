#include "bins/cache_bin.hpp"
#include <fmt/format.h>

namespace libmem {

cache_bin::cache_bin(std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz)

  : base_bin(segment_counter)
  , max_segsz_(max_segsz)
  , arena_name_(arena_name)
{
  this->handle_ = std::make_unique<libshm::shm_handle>(
    fmt::format("{}#cachbin", arena_name_), max_segsz_ * 4);
}

}
