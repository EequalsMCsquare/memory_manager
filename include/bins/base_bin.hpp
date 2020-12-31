#pragma once

#include <atomic>
#include <cstddef>
#include <mutex>

namespace libmem {
class base_bin
{
protected:
  std::atomic_size_t& segment_counter_;
  std::mutex          mtx_;

public:
  base_bin(std::atomic_size_t& segment_counter) noexcept;

  base_bin() = delete;

  base_bin(const base_bin&) = delete;

  /**
   * @brief allocate given size of memory segment
   *
   * @param nbytes
   * @return const int64_t memory segmen ptr
   */
  virtual const int64_t malloc(const size_t& nbytes) noexcept = 0;

  /**
   * @brief free a piece of segment
   *
   * @param ptr
   * @param nbytes
   * @return int 0: no error, -1 means segment is not in current range
   */
  virtual int free(const size_t& ptr, const size_t& nbytes) noexcept = 0;

  /**
   * @brief free all the segment
   *
   */
  virtual void clear() noexcept = 0;
};
} // namespace libmem
