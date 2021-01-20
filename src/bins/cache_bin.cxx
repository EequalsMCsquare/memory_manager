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
  , free_area_count_(BUFF_AREA_COUNT)
  , arena_name_(arena_name)
{
  spdlog::info("initializing cache bin");
  // create shm_handler
  this->handle_ = std::make_shared<shared_memory::shm_handle>(
    fmt::format("{}#cachbin", arena_name_),
    (sizeof(std::mutex) + max_segsz_ + sizeof(std::condition_variable)) *
      BUFF_AREA_COUNT);
  std::error_code ec;
  void*           __shm_buffer = this->handle_->map(ec);
  // check error_code
  if (ec) {
    spdlog::critical(
      "fail to construct cache bin. ({}) {}", ec.value(), ec.message());
    throw std::runtime_error("fail to construct cache bin.");
  }

  // initialize buffer area
  uint32_t i;

  std::condition_variable* __cond_ptr =
    reinterpret_cast<std::condition_variable*>(__shm_buffer);
  char* __area_ptr = reinterpret_cast<char*>(__cond_ptr + BUFF_AREA_COUNT);

  for (i = 0; i < BUFF_AREA_COUNT; i++) {
    this->area_condvs_[i] = new (__cond_ptr + i) std::condition_variable;
    this->area_buff_[i]   = __area_ptr + i * this->max_segsz_;
  }
  spdlog::info("cache bin initialized!");
}

std::future<int>
cache_bin::async_malloc(
  const size_t                                  nbytes,
  std::promise<std::shared_ptr<cache_segment>>& segment) noexcept
{

  // 思路:
  //    1. client 发送 < 1_KB的 allocate 请求
  //    2. 找到一片空闲shared buffering area
  //    3. 给这片buffering area 上锁
  //    4. How to achieve this?
  //    yield std::shared_ptr<base_segment>
  //    5. the buffering area's condition_variable.wait() until client copy the
  //    data into the buffering area.
  //    6. copy the shared memory buffering into local heap
  //    7. unlock
  //    8. done!

  // check which area is free.
  return std::async(std::launch::async, [&segment, nbytes, this]() -> int {
    // check if required size is <= max_segsz_
    if (nbytes > this->max_segsz_) {
      spdlog::error("required size must <= {}byte", this->max_segsz_);
      return -1;
    }
    uint32_t i;
    while (1) {
      for (i = 0; i < this->area_buff_.size(); i++) {
        if (this->area_mtx_[i].try_lock()) {
          // this area is free
          this->free_area_count_--;
          auto       __tmp = this->segment_counter_ref_++;
          auto       __seg = std::make_shared<cache_segment>();
          std::mutex __tmp_mtx;
          __seg->arena_name_   = this->arena_name_;
          __seg->size_         = nbytes;
          __seg->addr_pshift_  = buffarea_pshift(i);
          __seg->condv_pshift_ = condv_pshift(i);
          __seg->id_           = __tmp;
          // set promise
          spdlog::info("before set promise");
          segment.set_value(__seg);
          spdlog::info("after set promise");
          // allocate heap
          void* __heap_buffer = this->pmr_pool_.allocate(nbytes);
          if (__heap_buffer == nullptr) {
            spdlog::error("fail to allocate {} bytes", nbytes);
            this->area_mtx_[i].unlock();
            this->free_area_count_++;
            segment.set_exception(std::current_exception());
            return -1;
          }
          // wait for data to be copied to shared memory
          std::unique_lock<std::mutex> __ulock(__tmp_mtx);
          spdlog::info("before wait");
          this->area_condvs_[i]->wait(__ulock);
          spdlog::info("after wait.");
          // copy shared meory buffer to local heap
          std::memcpy(__heap_buffer, this->area_buff_[i], nbytes);
          this->area_mtx_[i].unlock();
          this->free_area_count_++;

          // track ptr;
          this->mtx_.lock();
          this->data_map_[__tmp] = { __heap_buffer, nbytes };
          this->mtx_.unlock();
          spdlog::info("before return");
          return 0;
        }
      }
      std::this_thread::sleep_for(10ms);
    }
  });
}

std::future<int>
cache_bin::async_retrieve(
  const size_t                                  segment_id,
  std::promise<std::shared_ptr<cache_segment>>& result) noexcept
{
  return std::async(std::launch::async, [&]() -> int {
    // try to find segment
    auto __iter = this->data_map_.find(segment_id);
    if (__iter == this->data_map_.end()) {
      spdlog::error("unable to locate the segment by given id, id: {}.",
                    segment_id);
      return -1;
    }
    auto     __pair = __iter->second;
    uint32_t i;
    while (1) {
      for (i = 0; i < this->area_buff_.size(); i++) {
        if (this->area_mtx_[i].try_lock()) {
          this->free_area_count_--;
          // when the area is free
          auto       __result_seg = std::make_shared<cache_segment>();
          std::mutex __tmp_mtx;
          std::unique_lock<std::mutex> __ulock(__tmp_mtx);
          // init result segment
          __result_seg->arena_name_   = this->arena_name_;
          __result_seg->size_         = __pair.second;
          __result_seg->addr_pshift_  = buffarea_pshift(i);
          __result_seg->condv_pshift_ = condv_pshift(i);
          __result_seg->id_           = segment_id;
          // copy data to buff area from heap
          std::memcpy(this->area_buff_[i], __pair.first, __pair.second);
          // fulfill promise
          result.set_value(__result_seg);
          // wait for client to retrieve data and notify
          this->area_condvs_[i]->wait(__ulock);
          // unlock area
          this->area_mtx_[i].unlock();
          this->free_area_count_++;
          return 0;
        }
      }
      std::this_thread::sleep_for(10ms);
    }
  });
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
cache_bin::buffarea_pshift(const uint32_t idx) const noexcept
{
  return BUFF_AREA_COUNT * sizeof(std::condition_variable) +
         this->max_segsz_ * idx;
}

size_t
cache_bin::condv_pshift(const uint32_t idx) const noexcept
{
  return idx * sizeof(std::condition_variable);
}

size_t
cache_bin::total_areas() const noexcept
{
  return this->area_buff_.size();
}
size_t
cache_bin::free_areas() const noexcept
{
  return this->free_area_count_;
}
size_t
cache_bin::area_size() const noexcept
{
  return this->max_segsz_;
}
std::shared_ptr<shared_memory::shm_handle>
cache_bin::get_shmhdl() const noexcept
{
  return this->handle_;
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
