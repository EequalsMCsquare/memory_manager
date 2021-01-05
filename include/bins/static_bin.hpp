#pragma once

#include <cstddef>
#include <vector>

#include "base_bin.hpp"

namespace libmem {
class static_bin : base_bin
{
private:
  const size_t      base_pshift_;
  const size_t      chunk_size_;
  const size_t      chunk_count_;
  size_t            chunk_left_;
  std::vector<bool> chunks_;

  /**
   * @brief Convert nbytes to how many chunks needed to allocate
   *
   * @param nbytes
   * @return const size_t
   */
  inline const size_t chunk_req(const size_t& nbytes) noexcept;

  std::vector<bool>::iterator first_fit(const size_t& nbytes) noexcept;
  // TODO: best fit

public:
  explicit static_bin(std::atomic_size_t& segment_counter,
                      const size_t&       chunk_size,
                      const size_t&       chunk_count,
                      const size_t&       base_pshift);

  std::shared_ptr<base_segment> malloc(const size_t nbytes) noexcept override;

  /**
   * @brief if free success, 0 will be returned.
   * -1 means ptr or segment is not in legal range.
   * -2 means one or more chunks is already marked as available.
   *
   * @param std::shared_ptr<base_segment>
   * @return int
   */
  int free(std::shared_ptr<base_segment> segment) noexcept override;

  void clear() noexcept override;

  const size_t base_pshift() noexcept;

  const size_t chunk_size() noexcept;

  const size_t chunk_count() noexcept;

  const size_t chunk_left() noexcept;
};
} // namespace libmem
