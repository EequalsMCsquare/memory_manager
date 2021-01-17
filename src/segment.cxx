#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>

namespace shm_kernel::memory_manager {

segmentdesc::segmentdesc(const stat_segment& seg)
  : seg_type(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
  , addr_pshift(seg.addr_pshift_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

segmentdesc::segmentdesc(const inst_segment& seg)
  : seg_type(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
  , addr_pshift(0)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

segmentdesc::segmentdesc(const cach_segment& seg)
  : seg_type(seg.type)
  , segment_id(seg.id_)
  , segment_size(seg.size_)
  , buff_pshift(seg.buff_pshift_)
  , condv_pshift(seg.condv_pshift_)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

std::string
segmentdesc::shmhdl_name() noexcept
{
  if (this->seg_type == SEG_TYPE::cachbin_segment) {
    return std::move(fmt::format("{}#cachbin", arena_name));
  } else if (this->seg_type == SEG_TYPE::instbin_segment) {
    return std::move(fmt::format("{}#instbin#seg{}", arena_name, segment_id));
  } else if (this->seg_type == SEG_TYPE::statbin_segment) {
    return std::move(fmt::format("{}#batch{}#statbin", arena_name, batch_id));
  }

  return {};
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