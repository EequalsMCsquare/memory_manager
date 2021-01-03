#pragma once

#include <cstddef>
#include <string_view>

namespace libmem {

enum class SEG_TYPE
{
  statbin_segment = 1,
  cachbin_segment = 2,
  instbin_segment = 3,
};

struct base_segment
{
  std::string_view arena_name_;
  const size_t     segment_id_;
  const size_t     batch_id_;
  const SEG_TYPE   type_;
  const size_t     bin_id_;
  const size_t     segment_size_;
};

struct statbin_segment : public base_segment
{
  const size_t addr_pshift;
};

struct cachbin_segment : public base_segment
{
  const size_t addr_pshift;
};

struct instbin_segment : public base_segment
{};

struct segmentdesc
{
  char           arena_name[256];
  const SEG_TYPE seg_type;
  const size_t   segment_id;
  const size_t   batch_id;
  const size_t   bin_id;
  const size_t   segment_size;
  const size_t   start_pshift;

  explicit segmentdesc(const instbin_segment& seg);

  explicit segmentdesc(const statbin_segment& seg);

  explicit segmentdesc(const cachbin_segment& seg);

  std::string shmhdl_name() noexcept;
};

}