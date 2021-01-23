#pragma once

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
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <utility>

#include "segment.hpp"

namespace shm_kernel::memory_manager {

constexpr uint32_t BUFF_AREA_COUNT = 8;

class cache_bin
{

protected:
  std::atomic_size_t&                    segment_counter_ref_;
  std::mutex                             mtx_;
  std::pmr::unsynchronized_pool_resource pmr_pool_;
  std::map<size_t /* segment id */, void* /* segment buffer */> data_map_;
  std::string_view                                              memmgr_name_;
  std::shared_ptr<spdlog::logger>                               logger;

public:
  explicit cache_bin(
    std::atomic_size_t&             segment_counter,
    std::string_view                memmgr_name,
    std::shared_ptr<spdlog::logger> logger = spdlog::default_logger());

  void set_logger(std::shared_ptr<spdlog::logger>);

  std::shared_ptr<cache_segment> store(void*        buffer,
                                       const size_t size) noexcept;

  void* retrieve(const size_t segment_id) noexcept;

  int set(const size_t segment_id, void* buffer, const size_t size) noexcept;

  int free(std::shared_ptr<cache_segment> segment) noexcept;

  void clear() noexcept;

  size_t segment_count() const noexcept;
};
}
