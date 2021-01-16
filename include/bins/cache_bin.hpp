#pragma once
#include "base_bin.hpp"
#include "shm_kernel/shared_memory.hpp"

#include <array>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <memory_resource>

/*
 *  cache bin buffer memory layout:
 *
 *  | condition_variable-0 | condition_variable-1 |
 *  | condition_variable-2 | condition_variable-3 |
 *  | buffer area-0 | buffer area-1  | buffer area-2 | buffer area-3 |
 */

namespace shm_kernel::memory_manager {

constexpr uint32_t BUFF_AREA_COUNT = 8;

enum class TASK_TYPE
{
  MALLOC,
  FREE,
};
class cache_bin : base_bin
{
private:
  std::pmr::unsynchronized_pool_resource                pmr_pool_;
  std::unique_ptr<shared_memory::shm_handle>            handle_;
  const size_t                                          max_segsz_;
  std::string_view                                      arena_name_;
  std::array<std::condition_variable*, BUFF_AREA_COUNT> area_condvs_;
  std::array<void*, BUFF_AREA_COUNT>                    area_buff_;
  std::array<std::mutex*, BUFF_AREA_COUNT>              area_mtx_;

public:
  explicit cache_bin(const size_t        id,
                     std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz);

  std::shared_ptr<base_segment> malloc(const size_t nbytes) noexcept override;

  int free(std::shared_ptr<base_segment> segment) noexcept override;

  void clear() noexcept override;
};
}