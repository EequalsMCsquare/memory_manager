#pragma once
#include "batch.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "mem_literals.hpp"
#include "segment.hpp"
#include "spdlog/logger.h"
#include <atomic>
#include <cstddef>
#include <mutex>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

/**
 * @brief Memory Manager Config
 *
 */

class mmgr
{

private:
  const std::string         name_;
  const std::vector<size_t> batch_bin_size_;
  const std::vector<size_t> batch_bin_count_;

  std::shared_ptr<spdlog::logger>                 _M_mmgr_logger;
  std::mutex                                      mtx_;
  std::shared_ptr<instant_bin>                    instant_bin_;
  std::shared_ptr<cache_bin>                      cache_bin_;
  std::vector<std::shared_ptr<batch>>             batches_;
  bool                                            is_initialized_;
  std::atomic_size_t                              segment_counter_{ 0 };
  std::map<size_t, std::shared_ptr<base_segment>> segment_table_;

  void PRE_CHECK() const;
  void init_INSTANT_BIN();
  void init_CACHE_BIN();

  // return new added batch sptr
  std::shared_ptr<batch> add_BATCH();

public:
  mmgr(const mmgr&) = delete;
  mmgr(mmgr&&)      = delete;
  mmgr()            = delete;

  mmgr(const std::string&         name,
       const std::vector<size_t>& batch_bin_size,
       const std::vector<size_t>& batch_bin_count,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());
  virtual ~mmgr();

  std::shared_ptr<cache_segment> cachbin_STORE(const void*  buffer,
                                               const size_t size) noexcept;
  int   cachbin_DEALLOC(const size_t segment_id) noexcept;
  void* cachbin_RETRIEVE(const size_t segment_id) noexcept;
  std::shared_ptr<instant_segment> instbin_ALLOC(const size_t size) noexcept;
  int instbin_DEALLOC(const size_t segment_id) noexcept;
  std::shared_ptr<static_segment> statbin_ALLOC(const size_t size) noexcept;
  int statbin_DEALLOC(const size_t segment_id) noexcept;
  std::shared_ptr<base_segment> get_segment(const size_t) noexcept;

  void                       set_logger(std::shared_ptr<spdlog::logger>);
  std::string_view           name() const noexcept;
  size_t                     segment_count() const noexcept;
  const std::vector<size_t>& batch_bin_size() const noexcept;
  const std::vector<size_t>& batch_bin_count() const noexcept;
};

}