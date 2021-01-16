#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <spdlog/spdlog.h>
#include <system_error>

#include "batch.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "segment.hpp"

namespace shm_kernel::memory_manager {

class arena
{
private:
  std::string                        name_;
  std::atomic_size_t                 segment_counter_; // segment_id
  std::deque<std::unique_ptr<batch>> batches_;
  std::unique_ptr<instant_bin>       instant_bin_;
  std::unique_ptr<cache_bin>         cache_bin_;

  void init_cache_bin();
  void init_instant_bin();
  void add_batch();

public:
  arena(std::string name);
  explicit arena() = delete;
  ~arena();
  std::shared_ptr<base_segment> allocate(const size_t nbytes);
  void             deallocate(std::shared_ptr<base_segment> segment);
  std::string_view name() noexcept;
  size_t           batch_count() noexcept;
};

} // namespace libmem