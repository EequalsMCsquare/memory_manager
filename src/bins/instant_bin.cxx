#include "bins/instant_bin.hpp"
#include <fmt/format.h>

namespace libmem {
instant_bin::instant_bin(std::atomic_size_t& segment_counter,
                         std::string_view    arena_name)
  : base_bin(segment_counter)
  , arena_name_(arena_name)
{}

const int64_t
instant_bin::malloc(const size_t& nbytes) noexcept
{
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGG(mtx_);
  size_t                      __tmp = this->segment_counter_ref_++;

  this->segments_.insert(std::make_pair(
    __tmp,
    std::make_shared<libshm::shm_handle>(
      fmt::format("{}#instbin#seg{}", arena_name_, __tmp), nbytes)));
  return std::move(__tmp);
}

int
instant_bin::free(const size_t& segment_id, const size_t& nbytes) noexcept
{
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG(mtx_);
  auto                        __pair = this->segments_.find(segment_id);
  if (__pair == this->segments_.end()) {
    return -1;
  }
  this->segments_.erase(__pair);
  return 0;
}

void
instant_bin::clear() noexcept
{
  this->segments_.clear();
}

const size_t
instant_bin::shmhdl_count() noexcept
{
  return this->segments_.size();
}
const size_t
instant_bin::size() noexcept
{
  return this->segments_.size();
}

std::shared_ptr<libshm::shm_handle>
instant_bin::get_shmhdl_ptr(const size_t& seg_id) noexcept
{
  auto __pair = this->segments_.find(seg_id);
  if (__pair == this->segments_.end()) {
    return {};
  }
  return __pair->second;
}
} // namespace libmem
