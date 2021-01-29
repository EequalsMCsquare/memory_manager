#pragma once

#include "bins/instant_bin.hpp"
#include "bins/static_bin.hpp"
#include "config.hpp"
#include "segment.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {
class batch
{
  friend class fmt::formatter<batch>;

protected:
  std::string_view    memmgr_name_;
  const size_t        id_;
  size_t              total_bytes_;
  std::atomic_size_t& segment_counter_ref_;

  std::unique_ptr<shared_memory::shm_handle> handle_;
  std::vector<std::unique_ptr<static_bin>>   static_bins_;
  std::shared_ptr<spdlog::logger>            _M_batch_logger;

  /**
   * @brief initialize shared memory handle for this batch
   *
   * @param buffsz shared memory handle size
   */
  void init_shm(const size_t& buffsz);

  explicit batch(std::string_view    arena_name,
                 const size_t&       id,
                 std::atomic_size_t& segment_counter,
                 std::shared_ptr<spdlog::logger> = spdlog::default_logger());
  /**
   * @brief
   *
   * @param statbin_chunksz
   * @param statbin_chunkcnt
   * @return size_t buffer size
   */
  size_t init_static_bins(const std::vector<size_t>& statbin_chunksz,
                          const std::vector<size_t>& statbin_chunkcnt);

public:
  explicit batch(std::string_view           arena_name,
                 const size_t&              id,
                 std::atomic_size_t&        segment_counter,
                 const std::vector<size_t>& statbin_chunksz,
                 const std::vector<size_t>& statbin_chunkcnt,
                 std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  explicit batch(std::string_view    arena_name,
                 const size_t&       id,
                 std::atomic_size_t& segment_counter,
                 const size_t&       statbin_minchunksz,
                 const size_t&       statbin_maxchunksz,
                 const size_t&       step,
                 const size_t&       statbin_size,
                 std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  explicit batch(std::string_view           arena_name,
                 const size_t&              id,
                 std::atomic_size_t&        segment_counter,
                 const std::vector<size_t>& statbin_chunksz,
                 const size_t&              statbin_chunkcnt,
                 std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  ~batch();

  /**
   * @brief allocate a shared memory segment.
   *
   * @param nbytes
   * @return std::shared_ptr<base_segment>
   */
  std::shared_ptr<static_segment> allocate(const size_t nbytes);

  /**
   * @brief deallocate a shared memory segment
   *
   * @param segment
   * @return int
   */
  int deallocate(std::shared_ptr<static_segment> segment) noexcept;

  std::string_view mmgr_name() noexcept;
  const size_t     id() noexcept;
  const size_t     max_chunksz() noexcept;
  const size_t     min_chunksz() noexcept;
  const size_t     total_bytes() noexcept;
};

}