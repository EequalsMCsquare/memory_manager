#include "bins/static_bin.hpp"
#include "config.hpp"
#include "ec.hpp"
#include "segment.hpp"

#include <algorithm>

namespace shm_kernel::memory_manager {

static_bin::static_bin(const size_t                    id,
                       std::atomic_size_t&             segment_counter,
                       const size_t&                   chunk_size,
                       const size_t&                   chunk_count,
                       const size_t&                   base_pshift,
                       std::shared_ptr<spdlog::logger> logger)
  : id_(id)
  , segment_counter_ref_(segment_counter)
  , base_pshift_(base_pshift)
  , chunk_size_(chunk_size)
  , chunk_count_(chunk_count)
  , chunks_(chunk_size_, true)
  , _M_statbin_logger(logger)
{
  logger->trace("正在初始化Static Bin...");
  logger->debug(
    "<static_bin>{{ID: {}, Chunk Size: {}, Chunk Count: {}, Base Pshift: {}.}}",
    id,
    chunk_size,
    chunk_count,
    base_pshift);
  if (this->chunk_size_ % ALIGNMENT != 0) {
    logger->critical("Chunk Size 必须对齐 {} bytes.", ALIGNMENT);
    throw std::runtime_error("chunk size must be aligned as " +
                             std::to_string(ALIGNMENT));
  }
  if (this->base_pshift_ % ALIGNMENT != 0) {
    logger->critical("Chunk的基础指针偏移必须对齐 {} bytes!", ALIGNMENT);
    throw std::runtime_error("base pshift must be aligned as " +
                             std::to_string(ALIGNMENT));
  }
  this->chunk_left_ = chunk_count;
  logger->trace("Static Bin 初始化完毕!");
}

std::vector<bool>::iterator
static_bin::first_fit(const size_t& chunks_req) noexcept
{
  auto __start = std::find_if(
    chunks_.begin(), chunks_.end(), [](const auto& tag) { return tag; });

  while (__start != chunks_.end()) {
    auto __end = std::find_if(
      __start, chunks_.end(), [](const auto& tag) { return !tag; });
    if (std::distance(__start, __end) < chunks_req) {
      __start =
        std::find_if(__end, chunks_.end(), [](const auto& tag) { return tag; });
    } else {
      return __start;
    }
  }

  // fail
  return chunks_.end();
}

std::shared_ptr<static_segment>
static_bin::malloc(const size_t nbytes, std::error_code& ec) noexcept
{
  ec.clear();
  size_t __segment_id = this->segment_counter_ref_++;
  // lock
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGG(this->mtx_);

  // cal how many chunks need to allocate
  auto __chunkreq = this->chunk_req(nbytes);
  // insufficient memory in this bin
  if (__chunkreq > this->chunk_left()) {
    _M_statbin_logger->error("当前Static Bin的内存不足以分配!");
    ec = MmgrErrc::NoMemory;
    return nullptr;
  }

  auto __iter = this->first_fit(__chunkreq);
  if (__iter == chunks_.end()) {
    ec = MmgrErrc::NoMemory;
    _M_statbin_logger->error("当前Static Bin的内存不足以分配!");
    return nullptr;
  }

  auto __seg = std::make_shared<static_segment>();
  // tag chunks as false which means not available
  std::for_each(__iter, __iter + __chunkreq, [](auto&& tag) { tag = false; });
  // decrease chunk_left;
  this->chunk_left_ -= __chunkreq;

  // cal ptr;
  size_t __ptr =
    std::distance(this->chunks_.begin(), __iter) * this->chunk_size();

  // assign to base_segment
  __seg->addr_pshift = __ptr + this->base_pshift();
  __seg->size        = nbytes;
  __seg->bin_id      = this->id();
  __seg->id          = __segment_id;
  return __seg;
}

int
static_bin::free(std::shared_ptr<static_segment> segment, std::error_code& ec) noexcept
{
  ec.clear();
  if (segment == nullptr) {
    ec = MmgrErrc::NullptrSegment;
    return -1;
  }
  // check if the segment is malloc in this bin
  if (segment->bin_id != this->id()) {
    ec = MmgrErrc::BinUnmatched;
    return -1;
  }
  auto __ptr  = segment->addr_pshift - this->base_pshift();
  auto __size = segment->size;

  auto __ptr_chunks = this->chunk_req(__ptr);

  // check ptr is in legal range
  if (__ptr_chunks > this->chunk_count()) {
    ec = MmgrErrc::IllegalSegmentRange;
    return -1;
  }

  auto __chunks = this->chunk_req(__size);
  // check segment is in legal range
  if (__ptr_chunks + __chunks > this->chunk_count()) {
    ec = MmgrErrc::IllegalSegmentRange;
    return -1;
  }
  // lock
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG(mtx_);

  // cal start chunk
  auto __start_chunk = chunks_.begin() + __ptr_chunks;
  // cal how many chunks
  auto __end_chunk = __start_chunk + __chunks;

  // check if the chunks are already marked as available.
  for (auto __iter = __start_chunk; __iter != __end_chunk; __iter++) {
    if (*__iter == true) {
      ec = MmgrErrc::SegmentDoubleFree;
      return -1;
    }
  }
  // mark the chunks available
  std::for_each(__start_chunk, __end_chunk, [](auto&& tag) { tag = true; });

  chunk_left_ += __chunks;
  return 0;
}

void
static_bin::clear() noexcept
{
  // lock
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGG(mtx_);

  std::for_each(chunks_.begin(), chunks_.end(), [](auto&& tag) { tag = true; });
  this->chunk_left_ = this->chunk_count();
}

size_t
static_bin::chunk_req(const size_t& nbytes) const noexcept
{
  return nbytes % chunk_size() == 0 ? nbytes / chunk_size()
                                    : nbytes / chunk_size() + 1;
}

const size_t
static_bin::id() const noexcept
{
  return this->id_;
}
const size_t
static_bin::base_pshift() const noexcept
{
  return this->base_pshift_;
}
const size_t
static_bin::chunk_size() const noexcept
{
  return this->chunk_size_;
}
const size_t
static_bin::chunk_count() const noexcept
{
  return this->chunk_count_;
}
const size_t
static_bin::chunk_left() const noexcept
{
  return this->chunk_left_;
}
} // namespace libmem
