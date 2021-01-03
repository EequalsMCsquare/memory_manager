#include "bins/instant_bin.hpp"

namespace libmem {
instant_bin::instant_bin(std::atomic_size_t& segment_counter,
                         std::string_view    arena_name)
  : base_bin(segment_counter)
  , arena_name_(arena_name)
{}
} // namespace libmem
