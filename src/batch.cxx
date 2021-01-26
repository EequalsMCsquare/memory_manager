#include "batch.hpp"
#include "config.hpp"
#include "segment.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

constexpr bool
is_aligned(const size_t& num)
{
  return num % ALIGNMENT == 0;
}

batch::batch(std::string_view                arena_name,
             const size_t&                   id,
             std::atomic_size_t&             segment_counter,
             std::shared_ptr<spdlog::logger> logger)
  : memmgr_name_(arena_name)
  , id_(id)
  , segment_counter_ref_(segment_counter)
  , logger(logger)
{
  logger->trace("initializing {}/batch{}", memmgr_name_, id_);
}

batch::batch(std::string_view                memmgr_name,
             const size_t&                   id,
             std::atomic_size_t&             segment_counter,
             const std::vector<size_t>&      statbin_chunksz,
             const std::vector<size_t>&      statbin_chunkcnt,
             std::shared_ptr<spdlog::logger> logger)
  : batch(memmgr_name, id, segment_counter, logger)
{
  logger->trace("正在初始化Batch...");
  if (statbin_chunkcnt.size() == 0) {
    logger->critical("Chunk Count不允许为空.");
    throw std::invalid_argument(
      "empty static bin chunk count is not acceptable.");
  }
  if (statbin_chunksz.size() == 0) {
    logger->critical("Chunk Size不允许为空.");
    throw std::invalid_argument(
      "empty static bin chunk size is not acceptable.");
  }
  if (statbin_chunkcnt.size() != statbin_chunksz.size()) {
    logger->critical("数组Chunk Size的长度和Chunk Count不一致!");
    throw std::invalid_argument("static bin chunk size's length should equal "
                                "to static bin chunk count's.");
  }

  this->init_shm(this->init_static_bins(statbin_chunksz, statbin_chunkcnt));
  logger->trace("{}/batch{} 初始化完毕!", memmgr_name_, id_);
}

batch::batch(std::string_view                arena_name,
             const size_t&                   id,
             std::atomic_size_t&             segment_counter,
             const size_t&                   statbin_minchunksz,
             const size_t&                   statbin_maxchunksz,
             const size_t&                   step,
             const size_t&                   statbin_size,
             std::shared_ptr<spdlog::logger> logger)
  : batch(arena_name, id, segment_counter, logger)
{
  logger->trace("正在初始化Batch...");
  if (statbin_minchunksz > statbin_maxchunksz) {
    logger->critical("Min Chunk Size 必须 <= Max Chunk Size");
    throw std::invalid_argument(
      "static bin min chunk must be smaller than max chunk");
  }
  if (!is_aligned(step)) {
    logger->critical("Step 必须以 {} 对齐!", ALIGNMENT);
    throw std::invalid_argument("step must be aligned to " +
                                std::to_string(ALIGNMENT));
  }
  if (!is_aligned(statbin_minchunksz) || !is_aligned(statbin_maxchunksz)) {
    logger->critical("Chunk Size 必须都要对齐 {}", ALIGNMENT);
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
  logger->trace("{}/batch{} 初始化完毕!", memmgr_name_, id_);
}

batch::batch(std::string_view                arena_name,
             const size_t&                   id,
             std::atomic_size_t&             segment_counter,
             const std::vector<size_t>&      statbin_chunksz,
             const size_t&                   statbin_chunkcnt,
             std::shared_ptr<spdlog::logger> logger)
  : batch(arena_name,
          id,
          segment_counter,
          statbin_chunksz,
          std::vector<size_t>(statbin_chunksz.size(), statbin_chunkcnt),
          logger)
{}

batch::~batch()
{
  logger->trace("destroying {}/batch{}", mmgr_name(), id());
}

size_t
batch::init_static_bins(const std::vector<size_t>& statbin_chunksz,
                        const std::vector<size_t>& statbin_chunkcnt)
{
  logger->trace("正在配置Static Bins...");
  // reserve
  this->static_bins_.reserve(statbin_chunksz.size());

  auto   __sz_iter        = statbin_chunksz.begin();
  auto   __cnt_iter       = statbin_chunkcnt.begin();
  size_t __idx            = 0;
  size_t __current_pshift = 0;

  // init this->static_bins_
  for (; __sz_iter != statbin_chunksz.end(); __sz_iter++, __cnt_iter++) {
    if (!is_aligned(*__sz_iter)) {
      logger->critical("Chunk Size 必须对齐 {} bytes", ALIGNMENT);
      throw std::invalid_argument("static bin chunk size must be aligned to " +
                                  std::to_string(ALIGNMENT));
    }
    this->static_bins_.push_back(
      std::make_unique<static_bin>(__idx++,
                                   this->segment_counter_ref_,
                                   *__sz_iter,
                                   *__cnt_iter,
                                   __current_pshift));
    __current_pshift += *__sz_iter * *__cnt_iter;
  }

  // desc sort
  std::sort(this->static_bins_.begin(),
            this->static_bins_.end(),
            [](const auto& a, const auto& b) {
              return a->chunk_size() > b->chunk_size();
            });
  this->total_bytes_ = __current_pshift;

  logger->trace("Static Bins 配置完毕!");
  return __current_pshift;
}

void
batch::init_shm(const size_t& buffsz)
{
  logger->trace("正在初始化shm_handle... size: {}KB", buffsz);
  auto handle_name = fmt::format("{}#batch{}#statbin", memmgr_name_, id_);
  try {
    this->handle_ =
      std::make_unique<shared_memory::shm_handle>(handle_name, buffsz);
  } catch (const std::exception& e) {
    logger->critical("无法创建shm_handle with following args: "
                     "{{handle_name: {}, buffer_size: {}}}. error message: {}",
                     handle_name.c_str(),
                     buffsz,
                     e.what());
    // re-throw
    throw e;
  }
  logger->trace("shm_handle 初始化完毕!");
}

std::shared_ptr<static_segment>
batch::allocate(const size_t nbytes)
{
  logger->trace("allocate {} bytes of segment", nbytes);
  // Too large, should've used instant bin
  if (nbytes > this->max_chunksz() * 8) {
    logger->error("allocate size too large for static bin, please consider "
                  "using instant bin instead! acceptable size should <= {}",
                  this->max_chunksz() * 8);
    // recoverable
    throw std::runtime_error(
      "required bytes too large! consider using instant bin instead.");
  }
  std::shared_ptr<static_segment> __segment;

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
        __segment->batch_id  = this->id();
        __segment->mmgr_name = this->mmgr_name();
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
      __segment->batch_id  = this->id();
      __segment->mmgr_name = this->mmgr_name();
      return std::move(__segment);
    }
  }
  // 没辙了, arena should push back a batch
  logger->warn(
    "unable to find a satified segment in {}/batch{}", mmgr_name(), id());
  return nullptr;
}

int
batch::deallocate(std::shared_ptr<static_segment> segment) noexcept
{
  if (segment->batch_id != this->id()) {
    logger->error(
      "segment->batch_id doesn't match batch's. expect {}, but received {}",
      this->id(),
      segment->batch_id);
    return -1;
  }
  if (segment->bin_id < this->static_bins_.size()) {
    for (const auto& bin : this->static_bins_) {
      if (bin->id() == segment->bin_id) {
        return bin->free(segment);
      }
    }
    logger->error("unalbe to find the allocate bin.");
    return -1;
  } else {
    logger->error("unalbe to find the allocate bin.");
    return -1;
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
batch::mmgr_name() noexcept
{
  return this->memmgr_name_;
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