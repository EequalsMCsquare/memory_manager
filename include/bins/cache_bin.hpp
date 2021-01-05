#pragma once
#include "base_bin.hpp"
#include "shm_kernel/shared_memory.hpp"

#include <deque>
#include <future>
#include <memory>
#include <memory_resource>

namespace libmem {

enum class TASK_TYPE
{
  MALLOC,
  FREE,
};
class cache_bin : base_bin
{
private:
  std::pmr::unsynchronized_pool_resource pmr_pool_;
  std::unique_ptr<libshm::shm_handle>    handle_;
  const size_t                           max_segsz_;
  std::string_view                       arena_name_;

public:
  explicit cache_bin(const size_t        id,
                     std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz);

  std::shared_ptr<base_segment> malloc(const size_t nbytes) noexcept override;

  int free(std::shared_ptr<base_segment> segment) noexcept override;

  void clear() noexcept override;
};
}