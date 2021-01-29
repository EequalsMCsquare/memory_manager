#include "bins/instant_bin.hpp"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {
instant_bin::instant_bin(std::atomic_size_t&             segment_counter,
                         std::string_view                memmgr_name,
                         std::shared_ptr<spdlog::logger> logger)
  : segment_counter_ref_(segment_counter)
  , memmgr_name_(memmgr_name)
  , _M_instbin_logger(logger)
{}

std::shared_ptr<instant_segment>
instant_bin::malloc(const size_t nbytes) noexcept
{
  size_t                      __tmp = this->segment_counter_ref_++;
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGG(mtx_);

  this->segments_.insert(std::make_pair(
    __tmp,
    std::make_shared<shared_memory::shm_handle>(
      fmt::format("{}#instbin#seg{}", memmgr_name_, __tmp), nbytes)));
  auto __seg       = std::make_shared<instant_segment>();
  __seg->mmgr_name = this->memmgr_name_;
  __seg->id        = __tmp;
  __seg->size      = nbytes;

  return std::move(__seg);
}

int
instant_bin::free(std::shared_ptr<instant_segment> segment) noexcept
{
  // TODO:
  // !这里有点问题, 应该先判断Segment的ref_count！
  if (segment->mmgr_name != this->memmgr_name_) {
    _M_instbin_logger->error("Segment 的memmgr名字与当前Instant Bin的不一致!");
    return -1;
  }
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG(mtx_);

  // search for segment's shm_handler
  auto __pair = this->segments_.find(segment->id);
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

std::shared_ptr<shared_memory::shm_handle>
instant_bin::get_shmhdl(const size_t& seg_id) noexcept
{
  auto __pair = this->segments_.find(seg_id);
  if (__pair == this->segments_.end()) {
    return {};
  }
  return __pair->second;
}
} // namespace libmem
