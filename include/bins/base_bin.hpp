#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>

#include "segment.hpp"
#include <spdlog/spdlog.h>

namespace libmem {
class base_bin
{
protected:
  std::atomic_size_t&             segment_counter_ref_;
  std::mutex                      mtx_;
  std::shared_ptr<spdlog::logger> logger_;

public:
  base_bin(std::atomic_size_t& segment_counter) noexcept;

  base_bin(std::atomic_size_t&             segment_counter,
           std::shared_ptr<spdlog::logger> logger) noexcept;

  base_bin() = delete;

  base_bin(const base_bin&) = delete;

  /**
   * @brief allocate given memory size segment
   *
   * @param nbytes
   * @return std::shared_ptr<base_segment>
   */
  virtual std::shared_ptr<base_segment> malloc(
    const size_t nbytes) noexcept = 0;

  /**
   * @brief free a piece of segment
   *
   * @param segment
   * @return int
   */
  virtual int free(std::shared_ptr<base_segment> segment) noexcept = 0;

  /**
   * @brief free all the segment
   *
   */
  virtual void clear() noexcept = 0;
};
} // namespace libmem
