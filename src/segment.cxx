#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>
#include <pstl/glue_algorithm_defs.h>
#include <stdexcept>

namespace shm_kernel::memory_manager {

void
segmentdesc::init_with_cache(const cache_segment& seg)
{
  this->segment_id   = seg.id;
  this->segment_size = seg.size;
  this->segment_type = seg.type;
  std::strncpy(this->mmgr_name, seg.mmgr_name.data(), 128);
}
void
segmentdesc::init_with_static(const static_segment& seg)
{
  this->segment_id   = seg.id;
  this->segment_size = seg.size;
  this->segment_type = seg.type;
  this->addr_pshift  = seg.addr_pshift;
  this->batch_id     = seg.batch_id;
  this->bin_id       = seg.bin_id;
  std::strncpy(this->mmgr_name, seg.mmgr_name.data(), 128);
}
void
segmentdesc::init_with_instant(const instant_segment& seg)
{
  this->segment_type = seg.type;
  this->segment_id   = seg.id;
  this->segment_size = seg.size;
  this->addr_pshift  = 0;
  std::strncpy(this->mmgr_name, seg.mmgr_name.data(), 128);
}

segmentdesc::segmentdesc(base_segment&& seg)
{
  if (seg.type == SEG_TYPE::cachbin_segment) {
    this->init_with_cache(dynamic_cast<cache_segment&>(seg));
  } else if (seg.type == SEG_TYPE::instbin_segment) {
    this->init_with_instant(dynamic_cast<instant_segment&>(seg));
  } else if (seg.type == SEG_TYPE::statbin_segment) {
    this->init_with_static(dynamic_cast<static_segment&>(seg));
  } else {
    throw std::runtime_error(fmt::format(
      "无法初始化！因为Segment的 typeid({})不合法", typeid(seg).name()));
  }
}

segmentdesc::segmentdesc(const static_segment& seg)
{
  this->init_with_static(seg);
}

segmentdesc::segmentdesc(std::shared_ptr<static_segment> seg)
{
  this->init_with_static(*seg.get());
}

segmentdesc::segmentdesc(const instant_segment& seg)
{
  this->init_with_instant(seg);
}

segmentdesc::segmentdesc(std::shared_ptr<instant_segment> seg)
{
  this->init_with_instant(*seg.get());
}

segmentdesc::segmentdesc(const cache_segment& seg)
{
  this->init_with_cache(seg);
}

segmentdesc::segmentdesc(std::shared_ptr<cache_segment> seg)
{
  this->init_with_cache(*seg.get());
}

std::string
segmentdesc::shmhdl_name() noexcept
{
  if (this->segment_type == SEG_TYPE::cachbin_segment) {
    return "";
  } else if (this->segment_type == SEG_TYPE::instbin_segment) {
    return fmt::format("{}#instbin#seg{}", mmgr_name, segment_id);
  } else if (this->segment_type == SEG_TYPE::statbin_segment) {
    return fmt::format("{}#batch{}#statbin", mmgr_name, batch_id);
  }

  return {};
}

segmentdesc
cache_segment::to_segmentdesc() const noexcept
{
  return segmentdesc(*this);
}

segmentdesc
instant_segment::to_segmentdesc() const noexcept
{
  return segmentdesc(*this);
}

segmentdesc
static_segment::to_segmentdesc() const noexcept
{
  return segmentdesc(*this);
}

cache_segment::cache_segment(std::string_view mmgr_name,
                             const size_t     id,
                             const size_t     size)
{
  this->mmgr_name = mmgr_name;
  this->id        = id;
  this->size      = size;
  this->type      = SEG_TYPE::cachbin_segment;
}

cache_segment::cache_segment()
{
  this->type = SEG_TYPE::cachbin_segment;
}

instant_segment::instant_segment(std::string_view mmgr_name,
                                 const size_t     id,
                                 const size_t     size)
{
  this->mmgr_name = mmgr_name;
  this->id        = id;
  this->size      = size;
  this->type      = SEG_TYPE::instbin_segment;
}

instant_segment::instant_segment()
{
  this->type = SEG_TYPE::instbin_segment;
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
  this->type        = SEG_TYPE::cachbin_segment;
  this->batch_id    = batch_id;
  this->bin_id      = bin_id;
  this->addr_pshift = addr_pshift;
}

static_segment::static_segment()
{
  this->type = SEG_TYPE::statbin_segment;
}

}

// template<typename FormatContext>
// auto
// fmt::formatter<libmem::SEG_TYPE>::format(libmem::SEG_TYPE type,
//                                          FormatContext&   ctx)
// {
//   std::string_view name = "unknown";
//   switch (type) {
//     case libmem::SEG_TYPE::cachbin_segment:
//       name = "Cache Bin";
//       break;
//     case libmem::SEG_TYPE::statbin_segment:
//       name = "Static Bin";
//       break;
//     case libmem::SEG_TYPE::instbin_segment:
//       name = "Instant Bin";
//       break;
//   }
//   return formatter<std::string_view>::format(name, ctx);
// }

// constexpr auto
// fmt::formatter<libmem::base_segment>::parse(format_parse_context& ctx)
// {
//   return ctx.end();
// }

// template<typename FormatContext>
// auto
// fmt::formatter<libmem::base_segment>::format(const libmem::base_segment& seg,
//                                              FormatContext&              ctx)
// {
//   return format_to(ctx.out(),
//                    "<base_segment>(alloc by: {}/batch{}/{}_{}, id: {}, size:
//                    "
//                    "{}, address shift: {})",
//                    seg.arena_name_,
//                    seg.batch_id_,
//                    seg.type_,
//                    seg.bin_id_,
//                    seg.id_,
//                    seg.size_,
//                    seg.addr_pshift_);
// }
