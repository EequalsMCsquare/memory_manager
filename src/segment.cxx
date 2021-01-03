#include "segment.hpp"

#include <cstring>
#include <fmt/format.h>

namespace libmem {

segmentdesc::segmentdesc(const instbin_segment& seg)
  : segment_id(seg.segment_id_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
  , segment_size(seg.segment_size_)
  , start_pshift(0)
  , seg_type(SEG_TYPE::instbin_segment)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

segmentdesc::segmentdesc(const statbin_segment& seg)
  : segment_id(seg.segment_id_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
  , segment_size(seg.segment_size_)
  , start_pshift(seg.addr_pshift)
  , seg_type(SEG_TYPE::statbin_segment)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

segmentdesc::segmentdesc(const cachbin_segment& seg)
  : segment_id(seg.segment_id_)
  , batch_id(seg.batch_id_)
  , bin_id(seg.bin_id_)
  , segment_size(seg.segment_size_)
  , start_pshift(seg.addr_pshift)
  , seg_type(SEG_TYPE::cachbin_segment)
{
  std::strncpy(this->arena_name, seg.arena_name_.data(), 256);
}

std::string
segmentdesc::shmhdl_name() noexcept
{
  if (this->seg_type == SEG_TYPE::cachbin_segment) {
    return std::move(
      fmt::format("{}#batch{}#bin{}", arena_name, batch_id, bin_id));
  } else if (this->seg_type == SEG_TYPE::instbin_segment) {
    return std::move(fmt::format(
      "{}#batch{}#bin{}#seg{}", arena_name, batch_id, bin_id, segment_id));
  } else if (this->seg_type == SEG_TYPE::statbin_segment) {
    return std::move(
      fmt::format("{}#batch{}#bin{}", arena_name, batch_id, bin_id));
  }

  return {};
}

}