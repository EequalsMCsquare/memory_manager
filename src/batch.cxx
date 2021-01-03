#include "batch.hpp"
#include <algorithm>

namespace libmem {

constexpr bool
is_aligned(const size_t& num)
{
  return num % ALIGNMENT == 0;
}

Batch::Batch(std::string_view                     arena_name,
             const size_t&                        id,
             const std::initializer_list<size_t>& statbin_chunksz,
             const std::initializer_list<size_t>& statbin_chunkcnt)
  : Batch(arena_name, id)
{
  if (statbin_chunkcnt.size() == 0) {
    throw std::invalid_argument(
      "empty static bin chunk count is not acceptable.");
  }
  if (statbin_chunksz.size() == 0) {
    throw std::invalid_argument(
      "empty static bin chunk size is not acceptable.");
  }
  if (statbin_chunkcnt.size() != statbin_chunksz.size()) {
    throw std::invalid_argument("static bin chunk size's length should equal "
                                "to static bin chunk count's.");
  }

  this->init_shm(this->init_static_bins(statbin_chunksz, statbin_chunkcnt));
}

Batch::Batch(std::string_view arena_name,
             const size_t&    id,
             const size_t&    statbin_minchunksz,
             const size_t&    statbin_maxchunksz,
             const size_t&    step,
             const size_t&    statbin_size)
  : Batch(arena_name, id)
{
  if (statbin_minchunksz > statbin_maxchunksz) {
    throw std::invalid_argument(
      "static bin min chunk must be smaller than max chunk");
  }
  if (!is_aligned(step)) {
    throw std::invalid_argument("step must be aligned to " +
                                std::to_string(ALIGNMENT));
  }
  if (!is_aligned(statbin_minchunksz) || !is_aligned(statbin_maxchunksz)) {
    throw std::invalid_argument("chunk size must be aligned to " +
                                std::to_string(ALIGNMENT));
  }
  size_t __bin_count = (statbin_maxchunksz - statbin_minchunksz) / step;
  size_t i;
  std::vector<size_t> __chunksz;
  __chunksz.reserve(__bin_count);
  for (i = 0; i < __bin_count; i++) {
    __chunksz.push_back(statbin_minchunksz + i * step);
  }
  this->init_shm(this->init_static_bins(
    __chunksz, std::vector<size_t>(__bin_count, statbin_size)));
}

Batch::Batch(std::string_view                     arena_name,
             const size_t&                        id,
             const std::initializer_list<size_t>& statbin_chunksz,
             const size_t&                        statbin_chunkcnt)
  : Batch(arena_name,
          id,
          statbin_chunksz,
          std::vector<size_t>(statbin_chunksz.size(), statbin_chunkcnt))
{}

size_t
Batch::init_static_bins(
  const std::initializer_list<size_t>& statbin_chunksz,
  const std::initializer_list<size_t>& statbin_chunkcnt) noexcpet
{
  // reserve
  this->static_bins_.reserve(statbin_chunksz.size());

  auto   __sz_iter        = statbin_chunksz.begin();
  auto   __cnt_iter       = statbin_chunkcnt.begin();
  size_t __current_pshift = 0;

  // init this->static_bins_
  for (; __sz_iter != statbin_chunksz.end(); __sz_iter++, __cnt_iter++) {
    if (!is_aligned(*__sz_iter)) {
      throw std::invalid_argument("static bin chunk size must be aligned to " +
                                  std::to_string(ALIGNMENT));
    }
    this->static_bins_.push_back(std::make_unique<static_bin>(
      this->segment_counter_ref_, *__sz_iter, *__cnt_iter, __current_pshift));
    __current_pshift += *__sz_iter * *__cnt_iter;
  }

  // desc sort
  std::sort(this->static_bins_.begin(),
            this->static_bins.end(),
            [](const auto& a, const auto& b) {
              return a.chunk_size() > b.chunk_size();
            });

  return std::move(__current_pshift);
}
}