#pragma once

#include <cstddef>
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
  size_t           size_;
};

/**
 * @brief segment allocated by cache bin
 *
 */
struct cache_segment : base_segment
{
  inline static SEG_TYPE type = SEG_TYPE::cachbin_segment;
  size_t                 condv_pshift_;
  size_t                 buff_pshift_;
};

struct instant_segment : base_segment
{
  inline static SEG_TYPE type = SEG_TYPE::instbin_segment;
};

struct static_segment : base_segment
{
  inline static SEG_TYPE type = SEG_TYPE::statbin_segment;
  size_t                 batch_id_;
  size_t                 bin_id_;
  size_t                 addr_pshift_;
};

struct segmentdesc
{
  char     arena_name[256];
  SEG_TYPE seg_type_;
  size_t   segment_id;
  size_t   segment_size;
  union
  {
    size_t addr_pshift;
    size_t buff_pshift;
  };
  union
  {
    size_t batch_id;
    size_t condv_pshift;
  };
  union
  {
    size_t    bin_id;
    std::byte __XXXXX_RESERVE__[sizeof(size_t)];
  };

  explicit segmentdesc(const cache_segment& seg);

  explicit segmentdesc(const static_segment& seg);

  explicit segmentdesc(const instant_segment& seg);

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