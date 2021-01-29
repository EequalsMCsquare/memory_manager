#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <string_view>

#include <shm_kernel/shared_memory.hpp>

#include "segment.hpp"

namespace shm_kernel::memory_manager {
class instant_bin
{
protected:
  std::atomic_size_t& segment_counter_ref_;
  std::mutex          mtx_;
  std::string_view    memmgr_name_;
  std::map<int, std::shared_ptr<shared_memory::shm_handle>> segments_;
  std::shared_ptr<spdlog::logger>                           _M_instbin_logger;

public:
  explicit instant_bin(
    std::atomic_size_t& segment_counter,
    std::string_view    memmgr_name,
    std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  instant_bin() = delete;

  instant_bin(const instant_bin&) = delete;

  std::shared_ptr<instant_segment> malloc(const size_t nbytes) noexcept;

  int free(std::shared_ptr<instant_segment> segment) noexcept;

  void clear() noexcept;

  const size_t shmhdl_count() noexcept;

  const size_t size() noexcept;

  std::shared_ptr<shared_memory::shm_handle> get_shmhdl(
    const size_t& seg_id) noexcept;
};
}