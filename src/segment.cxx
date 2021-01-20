#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>
#include <pstl/glue_algorithm_defs.h>

namespace shm_kernel::memory_manager {

segmentdesc::segmentdesc(const static_segment& seg)
  : seg_type_(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
  , addr_pshift(seg.addr_pshift_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 128);
}

segmentdesc::segmentdesc(const instant_segment& seg)
  : seg_type_(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
  , addr_pshift(0)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 128);
}

segmentdesc::segmentdesc(const cache_segment& seg)
  : seg_type_(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 128);
}

std::string
segmentdesc::shmhdl_name() noexcept
{
  if (this->seg_type_ == SEG_TYPE::cachbin_segment) {
    return fmt::format("{}#cachbin", arena_name);
  } else if (this->seg_type_ == SEG_TYPE::instbin_segment) {
    return fmt::format("{}#instbin#seg{}", arena_name, segment_id);
  } else if (this->seg_type_ == SEG_TYPE::statbin_segment) {
    return fmt::format("{}#batch{}#statbin", arena_name, batch_id);
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
