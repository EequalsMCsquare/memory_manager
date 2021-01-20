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
#include <mutex>
#include <shm_kernel/shared_memory.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <system_error>
#include <thread>

namespace shm_kernel::memory_manager {

using namespace std::chrono_literals;

cache_bin::cache_bin(std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz)

  : segment_counter_ref_(segment_counter)
  , max_segsz_(max_segsz)
  , arena_name_(arena_name)
{
  spdlog::info("initializing cache bin");

  spdlog::info("cache bin initialized!");
}

long
cache_bin::store(void* buffer, const size_t size) noexcept
{
  // check buffer
  if (buffer == nullptr) {
    spdlog::error("buffer ptr is nullptr!");
    return -1;
  }
  // check size
  if (this->max_segsz_ != 0) {
    if (size > this->max_segsz_) {
      spdlog::error("buffer size should <= {}.", this->max_segsz_);
      return -1;
    }
  }
  auto  __segment_id = this->segment_counter_ref_++;
  void* __alloc_buff = this->pmr_pool_.allocate(size);
  // check if allocate success
  if (__alloc_buff == nullptr) {
    spdlog::error("fail to allocate {} bytes", size);
    return -1;
  }
  // copy data to pool
  std::memcpy(__alloc_buff, buffer, size);
  // store it in data_map
  this->mtx_.lock();
  this->data_map_[__segment_id] = { __alloc_buff, size };
  this->mtx_.unlock();
  // return segment id
  return __segment_id;
}

void*
cache_bin::retrieve(const size_t segment_id, size_t& segment_size) noexcept
{
  // try to find the segment
  std::lock_guard __G(this->mtx_);
  if (auto __iter = this->data_map_.find(segment_id);
      __iter != this->data_map_.end()) {
    auto __pair  = __iter->second;
    segment_size = __pair.second;
    return __pair.first;
  }
  segment_size = 0;
  return nullptr;
}

int
cache_bin::set(const size_t segment_id,
               void*        buffer,
               const size_t size) noexcept
{
  // check buffer
  if (buffer == nullptr) {
    spdlog::error("buffer ptr is nullptr!");
    return -1;
  }
  // check size
  if (this->max_segsz_ != 0) {
    if (size > this->max_segsz_) {
      spdlog::error("buffer size should <= {}.", this->max_segsz_);
      return -1;
    }
  }
  // check if segment_id exist
  if (auto __iter = this->data_map_.find(segment_id);
      __iter != this->data_map_.end()) {
    // exist
    void* new_buffer = this->pmr_pool_.allocate(size);
    std::memcpy(new_buffer, buffer, size);
    return 0;
  }
  // not found
  return -1;
}

int
cache_bin::free(const size_t segment_id) noexcept
{
  // find ptr by segment->id_
  auto __iter = this->data_map_.find(segment_id);
  if (__iter == this->data_map_.end()) {
    spdlog::error("unable to locate the segment by id. id: {}", segment_id);
    return -1;
  }
  auto __pair = __iter->second;
  // deallocate heap buffer
  this->pmr_pool_.deallocate(__pair.first, __pair.second);
  // erase
  this->data_map_.erase(__iter);
  return 0;
}

void
cache_bin::clear() noexcept
{
  this->data_map_.clear();
  this->pmr_pool_.release();
}

size_t
cache_bin::area_size() const noexcept
{
  return this->max_segsz_;
}

size_t
cache_bin::max_segsz() const noexcept
{
  return this->max_segsz_;
}

size_t
cache_bin::segment_count() const noexcept
{
  return this->data_map_.size();
}
}
