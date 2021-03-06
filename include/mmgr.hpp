#pragma once
#include "batch.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "mem_literals.hpp"
#include "segment.hpp"
#include "spdlog/logger.h"
#include <atomic>
#include <cstddef>
#include <functional>
#include <mutex>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

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

  std::shared_ptr<cache_segment> CACHE_STORE(
    const size_t                      size,
    std::function<void(void* buffer)> callback,
    std::error_code&                  ec) noexcept;
  std::shared_ptr<cache_segment> CACHE_STORE(const void*      buffer,
                                               const size_t     size,
                                               std::error_code& ec) noexcept;
  std::shared_ptr<cache_segment> CACHE_STORE(const void*  buffer,
                                               const size_t size);

  int CACHE_DEALLOC(const size_t segment_id, std::error_code& ec) noexcept;
  int CACHE_DEALLOC(const size_t segment_id);

  int CACHE_SET(const size_t                      segment_id,
                  const size_t                      size,
                  std::function<void(void* buffer)> callback,
                  std::error_code&                  ec) noexcept;
  int CACHE_SET(const size_t     segment_id,
                  const void*      buffer,
                  const size_t     size,
                  std::error_code& ec) noexcept;
  int CACHE_SET(const size_t segment_id,
                  const void*  buffer,
                  const size_t size);

  void* CACHE_RETRIEVE(const size_t segment_id, std::error_code& ec) noexcept;
  void* CACHE_RETRIEVE(const size_t segment_id);

  std::shared_ptr<instant_segment> INSTANT_ALLOC(const size_t     size,
                                                 std::error_code& ec) noexcept;
  std::shared_ptr<instant_segment> INSTANT_ALLOC(const size_t size);

  int INSTANT_DEALLOC(const size_t segment_id, std::error_code& ec) noexcept;
  int INSTANT_DEALLOC(const size_t segment_id);

  std::shared_ptr<static_segment> STATIC_ALLOC(const size_t     size,
                                                std::error_code& ec) noexcept;
  std::shared_ptr<static_segment> STATIC_ALLOC(const size_t size);

  int STATIC_DEALLOC(const size_t segment_id, std::error_code& ec) noexcept;
  int STATIC_DEALLOC(const size_t segment_id);

  std::shared_ptr<base_segment> get_segment(const size_t     segment_id,
                                            std::error_code& ec) noexcept;
  std::shared_ptr<base_segment> get_segment(const size_t segment_id);

  void                       set_logger(std::shared_ptr<spdlog::logger>);
  std::string_view           name() const noexcept;
  size_t                     segment_count() const noexcept;
  const std::vector<size_t>& batch_bin_size() const noexcept;
  const std::vector<size_t>& batch_bin_count() const noexcept;
};

}