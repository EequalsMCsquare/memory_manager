#pragma once
#include "batch.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "mem_literals.hpp"
#include "segment.hpp"
#include "spdlog/logger.h"
#include <atomic>
#include <mutex>

namespace shm_kernel::memory_manager {

struct memmgr_config
{
  size_t              cache_bin_eps;
  size_t              instant_bin_eps;
  std::vector<size_t> batch_bin_size;
  std::vector<size_t> batch_bin_count;
};

class memmgr
{

private:
  std::shared_ptr<spdlog::logger> logger_;
  std::mutex                      mtx_;

  void init_INSTANT_BIN();
  void init_CACHE_BIN();
  void add_BATCH();

protected:
  bool                                is_initialized_;
  std::atomic_size_t                  segment_counter_{ 0 };
  std::shared_ptr<instant_bin>        instant_bin_;
  std::shared_ptr<cache_bin>          cache_bin_;
  std::vector<std::shared_ptr<batch>> batches_;
  memmgr_config                       config_;

  std::map<size_t, std::shared_ptr<base_segment>> allocated_segments_;

  std::shared_ptr<cache_segment>   cachbin_STORE(const size_t size,
                                                 const void*  buffer) noexcept;
  std::shared_ptr<instant_segment> instbin_ALLOC(const size_t size) noexcept;
  std::shared_ptr<static_segment>  statbin_ALLOC(const size_t size) noexcept;

  int instbin_DEALLOC(const size_t segment_id) noexcept;
  int statbin_DEALLOC(const size_t segment_id) noexcept;
  int cachbin_REMOVE(const size_t segment_id) noexcept;
  int cachbin_RELEASE(const size_t segment_id) noexcept;

  memmgr(const memmgr&) = delete;
  memmgr(memmgr&&)      = delete;
  memmgr()              = delete;
  explicit memmgr(memmgr_config&&);
  memmgr(memmgr_config&&, std::shared_ptr<spdlog::logger>);
  void init();
  void set_logger(std::shared_ptr<spdlog::logger>);
};

}