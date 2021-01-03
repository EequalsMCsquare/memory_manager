#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <system_error>

namespace libmem {

struct ArenaConfig
{
  size_t shm_eps;
  size_t insbin_eps;
};

class Arena
{
private:
  ArenaConfig        cfg_;
  std::atomic_size_t segment_counter_; // segment_id

public:
  explicit Arena();

  Arena(std::initializer_list<size_t>);

  Arena(ArenaConfig config);

  ~Arena();

  // TODO: allocate

  // TODO: deallocate
};

} // namespace libmem