#pragma once

#include "bins/static_bin.hpp"
#include "config.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

namespace libmem {
class Batch
{
private:
  std::string_view    arena_name_;
  const size_t        id_;
  std::atomic_size_t& segment_counter_ref_;

  // TODO: shm_handle

  // TODO: static_bin
  std::vector<std::unique_ptr<static_bin>> static_bins_;

  void init_shm(const size_t& buffsz);

  explicit Batch(std::string_view arena_name, const size_t& id);

  /**
   * @brief
   *
   * @param statbin_chunksz
   * @param statbin_chunkcnt
   * @return size_t buffer size
   */
  size_t init_static_bins(
    const std::initializer_list<size_t>& statbin_chunksz,
    const std::initializer_list<size_t>& statbin_chunkcnt) noexcept;

public:
  explicit Batch(std::string_view                     arena_name,
                 const size_t&                        id,
                 const std::initializer_list<size_t>& statbin_chunksz,
                 const std::initializer_list<size_t>& statbin_chunkcnt);

  explicit Batch(std::string_view arena_name,
                 const size_t&    id,
                 const size_t&    statbin_minchunksz,
                 const size_t&    statbin_maxchunksz,
                 const size_t&    step,
                 const size_t&    statbin_size);

  explicit Batch(std::string_view                     arena_name,
                 const size_t&                        id,
                 const std::initializer_list<size_t>& statbin_chunksz,
                 const size_t&                        statbin_chunkcnt);
};

}