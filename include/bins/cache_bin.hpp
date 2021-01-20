#pragma once
#include "shm_kernel/shared_memory.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <future>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <utility>

#include "segment.hpp"

namespace shm_kernel::memory_manager {

constexpr uint32_t BUFF_AREA_COUNT = 8;

class cache_bin
{

  using alloc_seg =
    std::pair<void* /* segment ptr */, size_t /* segment size */>;

protected:
  std::atomic_size_t&                    segment_counter_ref_;
  std::mutex                             mtx_;
  std::pmr::unsynchronized_pool_resource pmr_pool_;
  const size_t                           max_segsz_;
  std::map<size_t, alloc_seg>            data_map_;
  std::string_view                       arena_name_;

public:
  explicit cache_bin(std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz);

  long  store(void* buffer, const size_t size) noexcept;
  void* retrieve(const size_t segment_id, size_t& segment_size) noexcept;
  int   set(const size_t segment_id, void* buffer, const size_t size) noexcept;

  int free(const size_t segment_id) noexcept;

  void clear() noexcept;

  size_t area_size() const noexcept;

  size_t max_segsz() const noexcept;

  size_t segment_count() const noexcept;
};
}
