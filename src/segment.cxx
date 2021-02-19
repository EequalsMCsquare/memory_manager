#include "segment.hpp"
#include "except.hpp"
#include "mmgr.hpp"

#include <cstring>
#include <fmt/format.h>
#include <pstl/glue_algorithm_defs.h>
#include <stdexcept>
#include <variant>

namespace shm_kernel::memory_manager {

segment_info::segment_info(std::string_view mmgr_name,
                           const size_t     id,
                           const size_t     size,
                           SEG_TYPE         seg_type)
{
  if (mmgr_name.size() >= 128) {
    throw std::runtime_error("mmgr name is too long! size < 128 required!");
  }
  std::strncpy(this->mmgr_name_, mmgr_name.data(), 128);
  this->status_       = STATUS::CREATED;
  this->id_           = id;
  this->size_         = size;
  this->type_         = seg_type;
  this->local_buffer_ = nullptr;
}
segment_info::segment_info(std::string_view mmgr_name,
                           const size_t     id,
                           const size_t     size,
                           const SEG_TYPE   seg_type,
                           const size_t     addr_pshift,
                           const size_t     batch_id,
                           const size_t     bin_id)
  : segment_info(mmgr_name, id, size, seg_type)
{
  this->addr_pshift_  = addr_pshift;
  this->batch_id_     = batch_id;
  this->bin_id_       = bin_id;
  this->local_buffer_ = nullptr;
}

segment_info::segment_info(std::shared_ptr<cache_segment> segment)
  : segment_info(segment->mmgr_name,
                 segment->id,
                 segment->size,
                 SEG_TYPE::CACHE_SEGMENT)
{}

segment_info::segment_info(std::shared_ptr<static_segment> segment)
  : segment_info(segment->mmgr_name,
                 segment->id,
                 segment->size,
                 SEG_TYPE::STATIC_SEGMENT)
{
  this->batch_id_    = segment->batch_id;
  this->bin_id_      = segment->bin_id;
  this->addr_pshift_ = segment->addr_pshift;
}

segment_info::segment_info(std::shared_ptr<instant_segment> segment)
  : segment_info(segment->mmgr_name,
                 segment->id,
                 segment->size,
                 SEG_TYPE::INSTANT_SEGMENT)
{
  this->addr_pshift_ = 0;
}
char*
segment_info::ptr() const noexcept
{
  if (this->status_ != STATUS::REG) {
    return nullptr;
  } else {
    return static_cast<char*>(this->local_buffer_);
  }
}
segment_info::STATUS
segment_info::status() const noexcept
{
  return this->status_;
}

std::string_view
segment_info::mmgr_name() const noexcept
{
  return this->mmgr_name_;
}

size_t
segment_info::id() const noexcept
{
  return this->id_;
}

size_t
segment_info::size() const noexcept
{
  return this->size_;
}
SEG_TYPE
segment_info::type() const noexcept
{
  return this->type_;
}

void
segment_info::set_ptr(void* const ptr) noexcept
{
  this->local_buffer_ = ptr;
}
std::string
segment_info::shm_name() const noexcept
{
  // TODO:
  switch (this->type_) {
    case SEG_TYPE::CACHE_SEGMENT:
      return "";
    case SEG_TYPE::STATIC_SEGMENT:
      return fmt::format("{}#batch{}#statbin", mmgr_name(), batch_id_);
    case SEG_TYPE::INSTANT_SEGMENT:
      return fmt::format("{}#instbin#seg{}", mmgr_name(), id_);
  }
  return {};
}

cache_segment::cache_segment(std::string_view mmgr_name,
                             const size_t     id,
                             const size_t     size)
{
  this->mmgr_name = mmgr_name;
  this->id        = id;
  this->size      = size;
  this->type      = SEG_TYPE::CACHE_SEGMENT;
}
segment_info
cache_segment::to_seginfo() const noexcept
{
  return { mmgr_name, id, size, SEG_TYPE::CACHE_SEGMENT };
}

cache_segment::cache_segment()
{
  this->type = SEG_TYPE::CACHE_SEGMENT;
}

instant_segment::instant_segment(std::string_view mmgr_name,
                                 const size_t     id,
                                 const size_t     size)
{
  this->mmgr_name = mmgr_name;
  this->id        = id;
  this->size      = size;
  this->type      = SEG_TYPE::INSTANT_SEGMENT;
}
segment_info
instant_segment::to_seginfo() const noexcept
{
  return { mmgr_name, id, size, SEG_TYPE::INSTANT_SEGMENT };
}
instant_segment::instant_segment()
{
  this->type = SEG_TYPE::INSTANT_SEGMENT;
}

static_segment::static_segment(std::string_view mmgr_name,
                               const size_t     id,
                               const size_t     size,
                               const size_t     batch_id,
                               const size_t     bin_id,
                               const size_t     addr_pshift)
{
  this->mmgr_name   = mmgr_name;
  this->id          = id;
  this->size        = size;
  this->type        = SEG_TYPE::CACHE_SEGMENT;
  this->batch_id    = batch_id;
  this->bin_id      = bin_id;
  this->addr_pshift = addr_pshift;
}
segment_info
static_segment::to_seginfo() const noexcept
{
  return segment_info(mmgr_name, id, size, type, addr_pshift, batch_id, bin_id);
}
static_segment::static_segment()
{
  this->type = SEG_TYPE::STATIC_SEGMENT;
}

}
