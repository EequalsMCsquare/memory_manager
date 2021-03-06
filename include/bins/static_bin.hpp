#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <vector>

#include <spdlog/logger.h>

namespace shm_kernel::memory_manager {

class static_segment;
class static_bin
{
protected:
  const size_t                    id_;
  std::atomic_size_t&             segment_counter_ref_;
  std::mutex                      mtx_;
  const size_t                    base_pshift_;
  const size_t                    chunk_size_;
  const size_t                    chunk_count_;
  size_t                          chunk_left_;
  std::vector<bool>               chunks_;
  std::shared_ptr<spdlog::logger> _M_statbin_logger;

  /**
   * @brief Convert nbytes to how many chunks needed to allocate
   *
   * @param nbytes
   * @return const size_t
   */
  size_t chunk_req(const size_t& nbytes) const noexcept;

  std::vector<bool>::iterator first_fit(const size_t& nbytes) noexcept;

public:
  explicit static_bin(
    const size_t        id,
    std::atomic_size_t& segment_counter,
    const size_t&       chunk_size,
    const size_t&       chunk_count,
    const size_t&       base_pshift,
    std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  std::shared_ptr<static_segment> malloc(const size_t     nbytes,
                                         std::error_code& ec) noexcept;

  /**
   * @brief if free success, 0 will be returned.
   * -1 means ptr or segment is not in legal range.
   * -2 means one or more chunks is already marked as available.
   *
   * @param std::shared_ptr<base_segment>
   * @return int
   */
  int free(std::shared_ptr<static_segment> segment,
           std::error_code&                ec) noexcept;

  void clear() noexcept;

  const size_t id() const noexcept;

  const size_t base_pshift() const noexcept;

  const size_t chunk_size() const noexcept;

  const size_t chunk_count() const noexcept;

  const size_t chunk_left() const noexcept;
};
} // namespace libmem
