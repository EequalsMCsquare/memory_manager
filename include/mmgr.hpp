#pragma once
#include "batch.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "mem_literals.hpp"
#include "segment.hpp"
#include "spdlog/logger.h"
#include <atomic>
#include <mutex>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

/**
 * @brief Memory Manager Config
 *
 */
struct mmgr_config
{
  std::string         name;
  size_t              cache_bin_eps;
  size_t              instant_bin_eps;
  std::vector<size_t> batch_bin_size;
  std::vector<size_t> batch_bin_count;
};

class mmgr
{

private:
  std::shared_ptr<spdlog::logger>     _M_mmgr_logger;
  std::mutex                          mtx_;
  std::shared_ptr<instant_bin>        instant_bin_;
  std::shared_ptr<cache_bin>          cache_bin_;
  std::vector<std::shared_ptr<batch>> batches_;

  void check_CONFIG() const;
  void init_INSTANT_BIN();
  void init_CACHE_BIN();

  // return new added batch sptr
  std::shared_ptr<batch> add_BATCH();

protected:
  bool                                            is_initialized_;
  std::atomic_size_t                              segment_counter_{ 0 };
  const mmgr_config                               config_;
  std::map<size_t, std::shared_ptr<base_segment>> segment_table_;

  std::shared_ptr<cache_segment> cachbin_STORE(const size_t size,
                                               const void*  buffer) noexcept;
  int   cachbin_DEALLOC(const size_t segment_id) noexcept;
  void* cachbin_RETRIEVE(const size_t segment_id) noexcept;

  std::shared_ptr<instant_segment> instbin_ALLOC(const size_t size) noexcept;
  int instbin_DEALLOC(const size_t segment_id) noexcept;

  std::shared_ptr<static_segment> statbin_ALLOC(const size_t size) noexcept;
  int statbin_DEALLOC(const size_t segment_id) noexcept;

  mmgr(const mmgr&) = delete;
  mmgr(mmgr&&)      = delete;
  mmgr()            = delete;
  mmgr(mmgr_config&&,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());
  virtual ~mmgr();
  void set_logger(std::shared_ptr<spdlog::logger>);

public:
  std::string_view memmgr_name() const noexcept;
};

}