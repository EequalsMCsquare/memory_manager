#include "batch.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace libmem {

constexpr bool
is_aligned(const size_t& num)
{
  return num % ALIGNMENT == 0;
}

batch::batch(std::string_view arena_name, const size_t& id)
  : arena_name_(arena_name)
  , id_(id)
  , segment_counter_(0)
{
  this->logger_ = spdlog::stdout_color_mt(fmt::format("{}", arena_name_));
}

batch::batch(std::string_view                arena_name,
             const size_t&                   id,
             std::shared_ptr<spdlog::logger> logger)
  : arena_name_(arena_name)
  , id_(id)
  , segment_counter_(0)
  , logger_(logger)
{}

batch::batch(std::string_view           arena_name,
             const size_t&              id,
             const std::vector<size_t>& statbin_chunksz,
             const std::vector<size_t>& statbin_chunkcnt)
  : batch(arena_name, id)
{
  spdlog::info("initializing batch... name: {}, id: {}", arena_name_, id_);
  if (statbin_chunkcnt.size() == 0) {
    throw std::invalid_argument(
      "empty static bin chunk count is not acceptable.");
  }
  if (statbin_chunksz.size() == 0) {
    throw std::invalid_argument(
      "empty static bin chunk size is not acceptable.");
  }
  if (statbin_chunkcnt.size() != statbin_chunksz.size()) {
    throw std::invalid_argument("static bin chunk size's length should equal "
                                "to static bin chunk count's.");
  }

  this->init_shm(this->init_static_bins(statbin_chunksz, statbin_chunkcnt));
  spdlog::info("batch {} initializing complete.", id_);
}

batch::batch(std::string_view arena_name,
             const size_t&    id,
             const size_t&    statbin_minchunksz,
             const size_t&    statbin_maxchunksz,
             const size_t&    step,
             const size_t&    statbin_size)
  : batch(arena_name, id)
{
  spdlog::info("initializing batch... name: {}, id: {}", arena_name_, id_);
  if (statbin_minchunksz > statbin_maxchunksz) {
    throw std::invalid_argument(
      "static bin min chunk must be smaller than max chunk");
  }
  if (!is_aligned(step)) {
    throw std::invalid_argument("step must be aligned to " +
                                std::to_string(ALIGNMENT));
  }
  if (!is_aligned(statbin_minchunksz) || !is_aligned(statbin_maxchunksz)) {
    throw std::invalid_argument("chunk size must be aligned to " +
                                std::to_string(ALIGNMENT));
  }
  size_t __bin_count = (statbin_maxchunksz - statbin_minchunksz) / step;
  size_t i;
  std::vector<size_t> __chunksz;
  __chunksz.reserve(__bin_count);
  for (i = 0; i < __bin_count; i++) {
    __chunksz.push_back(statbin_minchunksz + i * step);
  }

  this->init_shm(this->init_static_bins(
    __chunksz, std::vector<size_t>(__bin_count, statbin_size)));
  spdlog::info("batch {} initializing complete.", id_);
}

batch::batch(std::string_view           arena_name,
             const size_t&              id,
             const std::vector<size_t>& statbin_chunksz,
             const size_t&              statbin_chunkcnt)
  : batch(arena_name,
          id,
          statbin_chunksz,
          std::vector<size_t>(statbin_chunksz.size(), statbin_chunkcnt))
{}

size_t
batch::init_static_bins(const std::vector<size_t>& statbin_chunksz,
                        const std::vector<size_t>& statbin_chunkcnt)
{
  // reserve
  this->static_bins_.reserve(statbin_chunksz.size());

  auto   __sz_iter        = statbin_chunksz.begin();
  auto   __cnt_iter       = statbin_chunkcnt.begin();
  size_t __current_pshift = 0;

  // init this->static_bins_
  for (; __sz_iter != statbin_chunksz.end(); __sz_iter++, __cnt_iter++) {
    if (!is_aligned(*__sz_iter)) {
      throw std::invalid_argument("static bin chunk size must be aligned to " +
                                  std::to_string(ALIGNMENT));
    }
    this->static_bins_.push_back(std::make_unique<static_bin>(
      this->segment_counter_, *__sz_iter, *__cnt_iter, __current_pshift));
    __current_pshift += *__sz_iter * *__cnt_iter;
  }

  // desc sort
  std::sort(this->static_bins_.begin(),
            this->static_bins_.end(),
            [](const auto& a, const auto& b) {
              return a->chunk_size() > b->chunk_size();
            });
  this->total_bytes_ = __current_pshift;

  return std::move(__current_pshift);
}

void
batch::init_shm(const size_t& buffsz)
{
  spdlog::info("initializing shm_handle... size: {}", buffsz);
  this->handle_ = std::make_unique<libshm::shm_handle>(
    fmt::format("{}#batch{}#statbin", arena_name_, id_), buffsz);
  spdlog::info("initializing shm_handle complete.");
}

std::shared_ptr<base_segment>
batch::allocate(const size_t nbytes)
{
  // Too large, should've used instant bin
  if (nbytes > this->max_chunksz() * 8) {
    throw std::runtime_error(
      "required bytes too large! consider using instant bin instead.");
  }
  std::shared_ptr<base_segment> __segment;

  std::vector<size_t> __rem;
  __rem.reserve(static_bins_.size());
  size_t i;
  for (i = 0; i < this->static_bins_.size(); i++) {
    auto __t_rem = nbytes % static_bins_[i]->chunk_size();
    // perfect match
    if (__t_rem == 0) {
      __segment = static_bins_[i]->malloc(nbytes);
      if (__segment == nullptr) {
        __rem.push_back(std::numeric_limits<size_t>::max());
        continue;
      } else {
        // perfect match available
        __segment->bin_id_     = i;
        __segment->batch_id_   = this->id();
        __segment->arena_name_ = this->arena_name();
        return std::move(__segment);
      }
    }
    __rem.push_back(std::move(__t_rem));
  }
  // if no perfect match, use the one with smallest remainder.
  size_t idx;
  for (i = 0; i < __rem.size(); i++) {
    auto min_iter = std::min_element(__rem.begin(), __rem.end());
    idx           = std::distance(__rem.begin(), min_iter);
    // change min_iter to max
    *min_iter = std::numeric_limits<size_t>::max();
    __segment = this->static_bins_[idx]->malloc(nbytes);
    if (__segment == nullptr) {
      // if fail to malloc with the bin, then fallback to next smallest
      // remainder bin.
      continue;
    } else {
      __segment->bin_id_     = idx;
      __segment->batch_id_   = this->id();
      __segment->arena_name_ = this->arena_name();
      return std::move(__segment);
    }
  }
  // 没辙了, arena should push back a batch
  return nullptr;
}

int
batch::deallocate(std::shared_ptr<base_segment> segment) noexcept
{
  if (segment->type_ != SEG_TYPE::statbin_segment) {
    return -1;
  }
  if (segment->bin_id_ >= 0 && segment->bin_id_ < this->static_bins_.size()) {
    // TODO: check segment is in the bin's range
    auto rv = this->static_bins_[segment->bin_id_]->free(segment);
    segment.reset();
    return rv;
  } else {
    return -2;
  }
}

const size_t
batch::max_chunksz() noexcept
{
  return this->static_bins_.front()->chunk_size();
}
const size_t
batch::min_chunksz() noexcept
{
  return this->static_bins_.back()->chunk_size();
}

std::string_view
batch::arena_name() noexcept
{
  return this->arena_name_;
}

const size_t
batch::id() noexcept
{
  return this->id_;
}

const size_t
batch::total_bytes() noexcept
{
  return this->total_bytes_;
}
}