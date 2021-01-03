#include "bins/base_bin.hpp"

namespace libmem {
base_bin::base_bin(std::atomic_size_t& segment_counter) noexcept
  : segment_counter_ref_(segment_counter)
{}
} // namespace libmem
