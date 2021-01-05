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

namespace libmem {
class batch
{
private:
  std::string_view   arena_name_;
  const size_t       id_;
  size_t             total_bytes_;
  std::atomic_size_t segment_counter_;

  std::unique_ptr<libshm::shm_handle>      handle_;
  std::vector<std::unique_ptr<static_bin>> static_bins_;
  std::shared_ptr<spdlog::logger>          logger_;

  void init_shm(const size_t& buffsz);

  explicit batch(std::string_view arena_name, const size_t& id);

  explicit batch(std::string_view                arena_name,
                 const size_t&                   id,
                 std::shared_ptr<spdlog::logger> logger);

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
                 const std::vector<size_t>& statbin_chunksz,
                 const std::vector<size_t>& statbin_chunkcnt);

  explicit batch(std::string_view arena_name,
                 const size_t&    id,
                 const size_t&    statbin_minchunksz,
                 const size_t&    statbin_maxchunksz,
                 const size_t&    step,
                 const size_t&    statbin_size);

  explicit batch(std::string_view           arena_name,
                 const size_t&              id,
                 const std::vector<size_t>& statbin_chunksz,
                 const size_t&              statbin_chunkcnt);

  ~batch();

  std::shared_ptr<base_segment> allocate(const size_t nbytes);

  int deallocate(std::shared_ptr<base_segment> segment) noexcept;

  std::string_view arena_name() noexcept;
  const size_t     id() noexcept;
  const size_t     max_chunksz() noexcept;
  const size_t     min_chunksz() noexcept;
  const size_t     total_bytes() noexcept;
};

}