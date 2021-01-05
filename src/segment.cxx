#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>

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