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
  size_t           id_;
  size_t           batch_id_;
  SEG_TYPE         type_;
  size_t           bin_id_;
  size_t           size_;
  size_t           addr_pshift_;
};

struct segmentdesc
{
  char           arena_name[256];
  const SEG_TYPE seg_type;
  const size_t   segment_id;
  const size_t   batch_id;
  const size_t   bin_id;
  const size_t   segment_size;
  const size_t   start_pshift;

  explicit segmentdesc(const base_segment& seg);

  std::string shmhdl_name() noexcept;
};

}