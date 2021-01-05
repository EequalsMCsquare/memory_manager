#include "bins/instant_bin.hpp"
#include <fmt/format.h>

namespace libmem {
instant_bin::instant_bin(const size_t        id,
                         std::atomic_size_t& segment_counter,
                         std::string_view    arena_name)
  : base_bin(id, segment_counter)
  , arena_name_(arena_name)
{}

std::shared_ptr<base_segment>
instant_bin::malloc(const size_t nbytes) noexcept
{
  size_t                      __tmp = this->segment_counter_ref_++;
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGG(mtx_);

  this->segments_.insert(std::make_pair(
    __tmp,
    std::make_shared<libshm::shm_handle>(
      fmt::format("{}#instbin#seg{}", arena_name_, __tmp), nbytes)));
  auto __seg          = std::make_shared<base_segment>();
  __seg->addr_pshift_ = 0;
  __seg->id_          = __tmp;
  __seg->size_        = nbytes;
  __seg->type_        = SEG_TYPE::instbin_segment;
  __seg->bin_id_      = this->id();

  return std::move(__seg);
}

int
instant_bin::free(std::shared_ptr<base_segment> segment) noexcept
{
  if (segment->type_ != SEG_TYPE::instbin_segment) {
    return -1;
  }
  if (segment->bin_id_ != this->id()) {
    return -1;
  }
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG(mtx_);
  auto                        __pair = this->segments_.find(segment->id_);
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
instant_bin::get_shmhdl(const size_t& seg_id) noexcept
{
  auto __pair = this->segments_.find(seg_id);
  if (__pair == this->segments_.end()) {
    return {};
  }
  return __pair->second;
}
} // namespace libmem
