#include "bins/cache_bin.hpp"
#include "segment.hpp"
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fmt/format.h>
#include <future>
#include <memory>
#include <mutex>
#include <shm_kernel/shared_memory.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <system_error>
#include <thread>

namespace shm_kernel::memory_manager {

using namespace std::chrono_literals;

cache_bin::cache_bin(std::atomic_size_t&             segment_counter,
                     std::string_view                memmgr_name,
                     std::shared_ptr<spdlog::logger> logger)
  : segment_counter_ref_(segment_counter)
  , mmgr_name_(memmgr_name)
  , _M_cachbin_logger(logger)
{
  logger->trace("正在初始化Cache bin...");

  logger->trace("Cache bin 初始化完毕!");
}

void
cache_bin::set_logger(std::shared_ptr<spdlog::logger> logger)
{
  this->_M_cachbin_logger = logger;
}

std::shared_ptr<cache_segment>
cache_bin::store(const void*      buffer,
                 const size_t     size,
                 std::error_code& ec) noexcept
{
  ec.clear();
  // check buffer
  if (buffer == nullptr) {
    _M_cachbin_logger->error("Buffer的指针不能为空指针!");
    ec = MmgrErrc::NullptrBuffer;
    return nullptr;
  }
  const size_t __tmp_id = this->segment_counter_ref_++;
  auto         __seg =
    std::make_shared<cache_segment>(this->mmgr_name_, __tmp_id, size);

  void* __alloc_buff = this->pmr_pool_.allocate(size);
  // check if allocate success
  if (__alloc_buff == nullptr) {
    ec = MmgrErrc::NoMemory;
    _M_cachbin_logger->error("分配{} bytes时失败!可能是内存不足", size);
    return nullptr;
  }
  // copy data to pool
  std::memcpy(__alloc_buff, buffer, size);
  // store it in data_map
  this->mtx_.lock();
  this->data_map_[__seg->id] = __alloc_buff;
  this->mtx_.unlock();
  // return segment
  return __seg;
}

void*
cache_bin::retrieve(const size_t segment_id, std::error_code& ec) noexcept
{
  ec.clear();
  // try to find the segment
  std::lock_guard __G(this->mtx_);
  if (auto __iter = this->data_map_.find(segment_id);
      __iter != this->data_map_.end()) {
    return __iter->second;
  }
  ec = MmgrErrc::SegmentNotFound;
  return nullptr;
}

int
cache_bin::free(std::shared_ptr<cache_segment> segment,
                std::error_code&               ec) noexcept
{
  ec.clear();
  // find ptr by segment->id_
  auto __iter = this->data_map_.find(segment->id);
  if (__iter == this->data_map_.end()) {
    _M_cachbin_logger->error(
      "没有在当前Cache bin中找到这个segment! segment id: {}", segment->id);
    ec = MmgrErrc::SegmentNotFound;
    return -1;
  }
  auto __pair = __iter->second;
  // deallocate heap buffer
  this->pmr_pool_.deallocate(__iter->second, segment->size);
  // erase
  this->data_map_.erase(__iter);
  return 0;
}

int
cache_bin::set(const size_t     segment_id,
               const size_t     origin_size,
               const void*      new_buffer,
               const size_t     new_size,
               std::error_code& ec) noexcept
{
  ec.clear();
  std::lock_guard<std::mutex> __lock(mtx_);
  auto                        __iter = this->data_map_.find(segment_id);
  if (__iter == this->data_map_.end()) {
    ec = MmgrErrc::SegmentNotFound;
    return -1;
  } else {
    this->pmr_pool_.deallocate(__iter->second, origin_size);
    void* __new_addr = this->pmr_pool_.allocate(new_size);
    std::memcpy(__new_addr, new_buffer, new_size);
    __iter->second = __new_addr;
    return 0;
  }
}

void
cache_bin::clear() noexcept
{
  this->data_map_.clear();
  this->pmr_pool_.release();
}

size_t
cache_bin::segment_count() const noexcept
{
  return this->data_map_.size();
}
}
