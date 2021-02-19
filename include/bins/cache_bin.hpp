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

// #include "segment.hpp"

namespace shm_kernel::memory_manager {

constexpr uint32_t BUFF_AREA_COUNT = 8;

class cache_segment;
class cache_bin
{

protected:
  std::atomic_size_t&                    segment_counter_ref_;
  std::mutex                             mtx_;
  std::pmr::unsynchronized_pool_resource pmr_pool_;
  std::map<size_t /* segment id */, void* /* segment buffer */> data_map_;
  std::string_view                                              mmgr_name_;
  std::shared_ptr<spdlog::logger> _M_cachbin_logger;

public:
  explicit cache_bin(
    std::atomic_size_t&             segment_counter,
    std::string_view                memmgr_name,
    std::shared_ptr<spdlog::logger> logger = spdlog::default_logger());

  void set_logger(std::shared_ptr<spdlog::logger>);

  std::shared_ptr<cache_segment> store(const void*      buffer,
                                       const size_t     size,
                                       std::error_code& ec) noexcept;

  std::shared_ptr<cache_segment> malloc(const size_t     size,
                                        void**           ptr,
                                        std::error_code& ec) noexcept;

  void* retrieve(const size_t segment_id, std::error_code& ec) noexcept;

  int free(std::shared_ptr<cache_segment> segment,
           std::error_code&               ec) noexcept;

  int free(std::shared_ptr<cache_segment> segment);

  void* realloc(std::shared_ptr<cache_segment> segment,
                const size_t                   new_size,
                std::error_code&               ec) noexcept;

  int set(const size_t     segment_id,
          const size_t     origin_size,
          const void*      new_buffer,
          const size_t     new_size,
          std::error_code& ec) noexcept;

  void clear() noexcept;

  size_t segment_count() const noexcept;
};
}
