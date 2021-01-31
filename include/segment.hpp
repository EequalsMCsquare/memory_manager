#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

namespace shm_kernel::memory_manager {

struct segmentdesc;

enum class SEG_TYPE
{
  statbin_segment = 1,
  cachbin_segment = 2,
  instbin_segment = 3,
};

struct base_segment
{
  std::string_view mmgr_name;
  size_t           id;
  size_t           size;
  SEG_TYPE         type;

  virtual segmentdesc to_segmentdesc() const noexcept = 0;
};

/**
 * @brief segment allocated by cache bin
 *
 */
struct cache_segment : base_segment
{
  cache_segment();
  cache_segment(std::string_view mmgr_name, const size_t id, const size_t size);
  segmentdesc to_segmentdesc() const noexcept override;
};

struct instant_segment : base_segment
{
  instant_segment();
  instant_segment(std::string_view mmgr_name,
                  const size_t     id,
                  const size_t     size);
  segmentdesc to_segmentdesc() const noexcept override;
};

struct static_segment : base_segment
{
  static_segment();
  static_segment(std::string_view mmgr_name,
                 const size_t     id,
                 const size_t     size,
                 const size_t     batch_id,
                 const size_t     bin_id,
                 const size_t     addr_pshift);

  size_t batch_id;
  size_t bin_id;
  size_t addr_pshift;

  segmentdesc to_segmentdesc() const noexcept override;
};

struct segmentdesc
{
  char     mmgr_name[128];
  SEG_TYPE segment_type;
  size_t   segment_id;
  size_t   segment_size;

  size_t addr_pshift;
  union
  {
    size_t    batch_id;
    std::byte __XXXX_RESERVE__[sizeof(size_t)];
  };
  union
  {
    size_t    bin_id;
    std::byte __XXXXX_RESERVE__[sizeof(size_t)];
  };

  void init_with_cache(const cache_segment&);
  void init_with_static(const static_segment&);
  void init_with_instant(const instant_segment&);
  segmentdesc() = default;
  segmentdesc(base_segment&&);
  segmentdesc(const cache_segment&);
  segmentdesc(std::shared_ptr<cache_segment>);
  segmentdesc(const static_segment&);
  segmentdesc(std::shared_ptr<static_segment>);
  segmentdesc(const instant_segment&);
  segmentdesc(std::shared_ptr<instant_segment>);

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
