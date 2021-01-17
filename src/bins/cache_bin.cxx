#include "bins/cache_bin.hpp"
#include "segment.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fmt/format.h>
#include <mutex>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <system_error>
#include <thread>

namespace shm_kernel::memory_manager {

using namespace std::chrono_literals;

cache_bin::cache_bin(const size_t        id,
                     std::atomic_size_t& segment_counter,
                     std::string_view    arena_name,
                     const size_t&       max_segsz)

  : id_(id)
  , segment_counter_ref_(segment_counter)
  , max_segsz_(max_segsz)
  , arena_name_(arena_name)
{
  spdlog::info("initializing cache bin");
  // create shm_handler
  this->handle_ = std::make_unique<shared_memory::shm_handle>(
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
  std::mutex* __mtx_ptr =
    reinterpret_cast<std::mutex*>(__cond_ptr + BUFF_AREA_COUNT);
  char* __area_ptr = reinterpret_cast<char*>(__mtx_ptr + BUFF_AREA_COUNT);

  for (i = 0; i < BUFF_AREA_COUNT; i++) {
    this->area_condvs_[i] = new (__cond_ptr + i) std::condition_variable;
    this->area_mtx_[i]    = new (__mtx_ptr + i) std::mutex;
    this->area_buff_[i]   = __area_ptr + i * this->max_segsz_;
  }
  spdlog::info("cache bin initialized!");
}

int
cache_bin::malloc(const size_t                nbytes,
                  std::promise<base_segment>& segment) noexcept
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

  // lock
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGG(this->mtx_);

  // check which area is free.
  uint32_t i;
  while (1) {
    for (i = 0; i < this->area_buff_.size(); i++) {
      if (this->area_mtx_[i]->try_lock()) {
        // this area is free

        // TODO:
      }
    }
    std::this_thread::sleep_for(10ms);
  }
}

int
cache_bin::free(std::shared_ptr<base_segment> segment) noexcept
{
  // TODO:
  return 0;
}

void
cache_bin::clear() noexcept
{
  // TODO:
}

const size_t
cache_bin::id() const noexcept
{
  return this->id_;
}
}
