#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <variant>

namespace shm_kernel::memory_manager {

class segment_info;

enum class SEG_TYPE
{
  STATIC_SEGMENT  = 1,
  CACHE_SEGMENT   = 2,
  INSTANT_SEGMENT = 3,
};
struct base_segment
{
  std::string_view mmgr_name;
  size_t           id;
  size_t           size;
  SEG_TYPE         type;

  virtual segment_info to_seginfo() const noexcept = 0;
};

struct cache_segment : base_segment
{
  cache_segment();
  cache_segment(std::string_view mmgr_name, const size_t id, const size_t size);

  segment_info to_seginfo() const noexcept override final;
};
struct instant_segment : base_segment
{
  instant_segment();
  instant_segment(std::string_view mmgr_name,
                  const size_t     id,
                  const size_t     size);
  segment_info to_seginfo() const noexcept override final;
};
struct static_segment : base_segment
{
  size_t batch_id;
  size_t bin_id;
  size_t addr_pshift;

  static_segment();
  static_segment(std::string_view mmgr_name,
                 const size_t     id,
                 const size_t     size,
                 const size_t     batch_id,
                 const size_t     bin_id,
                 const size_t     addr_pshift);

  segment_info to_seginfo() const noexcept override final;
};

/**
 * @brief every thing need to locate a segment
 *
 */
class segment_info
{

  friend class static_segment;
  friend class cache_segment;
  friend class instant_segment;

private:
  char     mmgr_name_[128];
  size_t   id_;
  size_t   size_;
  SEG_TYPE type_;
  union
  {
    size_t addr_pshift_;
    void*  cache_buffer_;
  };
  size_t batch_id_;
  size_t bin_id_;

  void set_ptr(void* const ptr) noexcept;

  segment_info(std::string_view mmgr_name,
               const size_t     id,
               const size_t     size,
               SEG_TYPE         seg_type);

  segment_info(std::string_view mmgr_name,
               const size_t     id,
               const size_t     size,
               const SEG_TYPE   seg_type,
               const size_t     addr_pshift,
               const size_t     batch_id,
               const size_t     bin_id);

public:
  // cache segment constructor
  segment_info(std::shared_ptr<cache_segment>);

  // static segment constructor
  segment_info(std::shared_ptr<static_segment>);

  // instant segment constructor
  segment_info(std::shared_ptr<instant_segment>);

  std::string_view            mmgr_name() const noexcept;
  size_t                      id() const noexcept;
  size_t                      size() const noexcept;
  SEG_TYPE                    type() const noexcept;
  std::variant<size_t, void*> ptr() const noexcept;
};

}