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

/*
 *  cache bin buffer memory layout:
 *
 *  | condition_variable-0 | condition_variable-1 |
 *  | condition_variable-2 | condition_variable-3 |
 *  | buffer area-0 | buffer area-1  | buffer area-2 | buffer area-3 |
 */

namespace shm_kernel::memory_manager {

constexpr uint32_t BUFF_AREA_COUNT = 8;

class cache_bin
{

  using alloc_seg =
    std::pair<void* /* segment ptr */, size_t /* segment size */>;

protected:
  std::atomic_size_t&                                   segment_counter_ref_;
  std::mutex                                            mtx_;
  std::pmr::unsynchronized_pool_resource                pmr_pool_;
  std::shared_ptr<shared_memory::shm_handle>            handle_;
  const size_t                                          max_segsz_;
  std::atomic_size_t                                    free_area_count_;
  std::string_view                                      arena_name_;
  std::array<std::condition_variable*, BUFF_AREA_COUNT> area_condvs_;
  std::array<void*, BUFF_AREA_COUNT>                    area_buff_;
  std::array<std::mutex, BUFF_AREA_COUNT>               area_mtx_;
  std::map<size_t, alloc_seg>                           data_map_;
  size_t buffarea_pshift(const uint32_t idx) const noexcept;
  size_t condv_pshift(const uint32_t idx) const noexcept;

public:
  explicit cache_bin(std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz);

  std::future<int> async_malloc(
    const size_t                                  nbytes,
    std::promise<std::shared_ptr<cache_segment>>& segment) noexcept;

  std::future<int> async_retrieve(
    const size_t                                  segment_id,
    std::promise<std::shared_ptr<cache_segment>>& result) noexcept;

  int free(const size_t segment_id) noexcept;

  void clear() noexcept;

  size_t total_areas() const noexcept;

  size_t free_areas() const noexcept;

  size_t area_size() const noexcept;

  size_t max_segsz() const noexcept;

  size_t segment_count() const noexcept;

  std::shared_ptr<shared_memory::shm_handle> get_shmhdl() const noexcept;
};
}