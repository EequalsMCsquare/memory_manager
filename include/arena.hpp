#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <spdlog/spdlog.h>
#include <system_error>

#include "batch.hpp"
#include "bins/instant_bin.hpp"
#include "segment.hpp"

namespace libmem {

class arena
{
private:
  std::string                        name_;
  std::atomic_size_t                 segment_counter_; // segment_id
  std::deque<std::unique_ptr<batch>> batches_;
  std::unique_ptr<instant_bin>       instant_bin_;
  // TODO: cache_bin

  void init_cache_bin();
  void init_instant_bin();
  void add_batch();

public:
  explicit arena() = delete;
  arena(std::string name);

  ~arena();

  // TODO: allocate
  std::shared_ptr<base_segment> allocate(const size_t nbytes);

  // TODO: deallocate
  void deallocate(std::shared_ptr<base_segment> segment);

  std::string_view name() noexcept;
  size_t           batch_count() noexcept;
};

} // namespace libmem