#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>
#include <spdlog/fmt/bundled/core.h>

namespace libmem {

segmentdesc::segmentdesc(const base_segment& seg)
  : segment_id(seg.id_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
  , segment_size(seg.size_)
  , start_pshift(seg.addr_pshift_)
  , seg_type(seg.type_)
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

template<>
struct fmt::formatter<libmem::SEG_TYPE> : formatter<std::string_view>
{
  template<typename FormatContext>
  auto format(libmem::SEG_TYPE type, FormatContext& ctx)
  {
    std::string_view name = "unknown";
    switch (type) {
      case libmem::SEG_TYPE::cachbin_segment:
        name = "Cache Bin";
        break;
      case libmem::SEG_TYPE::statbin_segment:
        name = "Static Bin";
        break;
      case libmem::SEG_TYPE::instbin_segment:
        name = "Instant Bin";
        break;
    }
    return formatter<std::string_view>::format(name, ctx);
  }
};

template<>
struct fmt::formatter<libmem::base_segment>
{
  constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

  template<typename FormatContext>
  auto format(const libmem::base_segment& seg, FormatContext& ctx)
  {
    return format_to(ctx.out(),
                     "<base_segment>(alloc by: {}/batch{}/{}_{}, id: {}, size: "
                     "{}, address shift: {})",
                     seg.arena_name_,
                     seg.batch_id_,
                     seg.type_,
                     seg.bin_id_,
                     seg.id_,
                     seg.size_,
                     seg.addr_pshift_);
  }
};