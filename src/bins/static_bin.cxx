#include "bins/static_bin.hpp"
#include "config.hpp"

#include <algorithm>

namespace shm_kernel::memory_manager {

static_bin::static_bin(const size_t        id,
                       std::atomic_size_t& segment_counter,
                       const size_t&       chunk_size,
                       const size_t&       chunk_count,
                       const size_t&       base_pshift)
  : id_(id)
  , segment_counter_ref_(segment_counter)
  , base_pshift_(base_pshift)
  , chunk_size_(chunk_size)
  , chunk_count_(chunk_count)
  , chunks_(chunk_size_, true)
{
  if (this->chunk_size_ % ALIGNMENT != 0) {
    throw std::runtime_error("chunk size must be aligned as " +
                             std::to_string(ALIGNMENT));
  }
  if (this->base_pshift_ % ALIGNMENT != 0) {
    throw std::runtime_error("base pshift must be aligned as " +
                             std::to_string(ALIGNMENT));
  }
  this->chunk_left_ = chunk_count;
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

std::shared_ptr<base_segment>
static_bin::malloc(const size_t nbytes) noexcept
{
  size_t __segment_id = this->segment_counter_ref_++;
  // lock
  std::lock_guard<std::mutex> GGGGGGGGGGGGGGGGGGGGGGGG(this->mtx_);

  // cal how many chunks need to allocate
  auto __chunkreq = this->chunk_req(nbytes);
  // insufficient memory in this bin
  if (__chunkreq > this->chunk_left()) {
    return nullptr;
  }

  auto __iter = this->first_fit(__chunkreq);
  if (__iter == chunks_.end()) {
    return nullptr;
  }

  auto __seg = std::make_shared<base_segment>();
  // tag chunks as false which means not available
  std::for_each(__iter, __iter + __chunkreq, [](auto&& tag) { tag = false; });
  // decrease chunk_left;
  this->chunk_left_ -= __chunkreq;

  // cal ptr;
  size_t __ptr =
    std::distance(this->chunks_.begin(), __iter) * this->chunk_size();

  // assign to base_segment
  __seg->addr_pshift_ = __ptr + this->base_pshift();
  __seg->size_        = nbytes;
  __seg->type_        = SEG_TYPE::statbin_segment;
  __seg->bin_id_      = this->id();
  __seg->id_          = __segment_id;
  return __seg;
}

int
static_bin::free(std::shared_ptr<base_segment> segment) noexcept
{
  if (segment == nullptr) {
    return -1;
  }
  // check if the segment type is statbin_segment
  if (segment->type_ != SEG_TYPE::statbin_segment) {
    return -1;
  }
  // check if the segment is malloc in this bin
  if (segment->bin_id_ != this->id()) {
    return -1;
  }
  auto __ptr  = segment->addr_pshift_ - this->base_pshift();
  auto __size = segment->size_;

  auto __ptr_chunks = this->chunk_req(__ptr);

  // check ptr is in legal range
  if (__ptr_chunks > this->chunk_count()) {
    return -1;
  }

  auto __chunks = this->chunk_req(__size);
  // check segment is in legal range
  if (__ptr_chunks + __chunks > this->chunk_count()) {
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
      return -2;
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

const size_t
static_bin::chunk_req(const size_t& nbytes) noexcept
{
  return std::move(nbytes % chunk_size() == 0 ? nbytes / chunk_size()
                                              : nbytes / chunk_size() + 1);
}

const size_t
static_bin::id() noexcept
{
  return this->id_;
}
const size_t
static_bin::base_pshift() noexcept
{
  return this->base_pshift_;
}
const size_t
static_bin::chunk_size() noexcept
{
  return this->chunk_size_;
}
const size_t
static_bin::chunk_count() noexcept
{
  return this->chunk_count_;
}
const size_t
static_bin::chunk_left() noexcept
{
  return this->chunk_left_;
}
} // namespace libmem
