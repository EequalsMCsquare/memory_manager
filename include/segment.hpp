#pragma once

#include <cstddef>
#include <fmt/format.h>
#include <string_view>

namespace shm_kernel::memory_manager {

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

// template<>
// struct fmt::formatter<libmem::SEG_TYPE> : formatter<std::string_view>
// {
//   template<typename FormatContext>
//   auto format(libmem::SEG_TYPE, FormatContext&);
// };

// template<>
// struct fmt::formatter<libmem::base_segment>
// {
//   constexpr auto parse(format_parse_context& ctx);

//   template<typename FormatContext>
//   auto format(const libmem::base_segment&, FormatContext& ctx);
// };