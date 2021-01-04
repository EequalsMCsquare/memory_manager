#pragma once

#include "base_bin.hpp"
#include <map>
#include <memory>
#include <string_view>

#include <shm_kernel/shared_memory.hpp>

namespace libmem {
class instant_bin : base_bin
{
private:
  std::string_view                                   arena_name_;
  std::map<int, std::shared_ptr<libshm::shm_handle>> segments_;

public:
  explicit instant_bin(std::atomic_size_t& segment_counter,
                       std::string_view    arena_name);

  instant_bin() = delete;

  instant_bin(const instant_bin&) = delete;

  /**
   * @brief
   *
   * @param nbytes
   * @return const int64_t segment_id
   */
  const int64_t malloc(const size_t& nbytes) noexcept override;

  int free(const size_t& segment_id,
           const size_t& nbytes = 0) noexcept override;

  void clear() noexcept override;

  const size_t shmhdl_count() noexcept;

  const size_t size() noexcept;

  std::shared_ptr<libshm::shm_handle> get_shmhdl_ptr(
    const size_t& seg_id) noexcept;
};
}