#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <system_error>

#include <spdlog/spdlog.h>

namespace libmem {

struct ArenaConfig
{
  size_t shm_eps;
  size_t insbin_eps;
};

class Arena
{
private:
  ArenaConfig                     cfg_;
  std::atomic_size_t              segment_counter_; // segment_id
  std::shared_ptr<spdlog::logger> logger_;

public:
  explicit Arena();
  Arena(std::shared_ptr<spdlog::logger> logger);
  Arena(ArenaConfig config);
  Arena(std::initializer_list<size_t>);

  ~Arena();

  // TODO: allocate

  // TODO: deallocate
};

} // namespace libmem