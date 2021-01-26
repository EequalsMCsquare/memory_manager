#include "mmgr.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "segment.hpp"
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

mmgr::mmgr(mmgr_config&& config, std::shared_ptr<spdlog::logger> logger)
  : config_(config)
  , logger_(logger)
{
  logger_->trace("正在初始化Memory Manager...");
  this->check_CONFIG();
  this->init_INSTANT_BIN();
  this->init_CACHE_BIN();
  this->add_BATCH();
  logger->trace("Memory Manager 初始化完毕!");
}

void
mmgr::check_CONFIG() const
{
  // TODO
}

void
mmgr::init_INSTANT_BIN()
{
  this->instant_bin_ = std::make_shared<instant_bin>(
    this->segment_counter_, this->memmgr_name(), this->logger_);
}

void
mmgr::init_CACHE_BIN()
{
  this->cache_bin_ =
    std::make_shared<cache_bin>(segment_counter_, memmgr_name(), this->logger_);
}

std::shared_ptr<batch>
mmgr::add_BATCH()
{
  std::lock_guard<std::mutex> GG(this->mtx_);
  this->batches_.push_back(std::make_shared<batch>(this->memmgr_name(),
                                                   batches_.size(),
                                                   segment_counter_,
                                                   config_.batch_bin_size,
                                                   config_.batch_bin_count,
                                                   this->logger_));
  return this->batches_.back();
}

std::shared_ptr<cache_segment>
mmgr::cachbin_STORE(const size_t size, const void* buffer) noexcept
{
  if (buffer == nullptr) {
    logger_->error("Buffer 不能为空指针!");
    return nullptr;
  }
  if (size > this->config_.cache_bin_eps) {
    logger_->error("Size 不能大于Cache Bin Eps({})!", config_.cache_bin_eps);
    return nullptr;
  }
  return this->cache_bin_->store(buffer, size);
}

std::shared_ptr<instant_segment>
mmgr::instbin_ALLOC(const size_t size) noexcept
{
  if (size < this->config_.instant_bin_eps) {
    logger_->error("Size 不能小于 Instant Bin Eps({})",
                   config_.instant_bin_eps);
    return nullptr;
  }
  return this->instant_bin_->malloc(size);
}

std::shared_ptr<static_segment>
mmgr::statbin_ALLOC(const size_t size) noexcept
{
  if (size <= this->config_.cache_bin_eps) {
    logger_->error("Size 不能小于Cache Bin Eps({})", config_.cache_bin_eps);
    return nullptr;
  }
  std::shared_ptr<static_segment> __seg;
  for (const auto& batch : batches_) {
    __seg = batch->allocate(size);
    if (__seg) {
      // if allocate success, break loop
      break;
    }
  }
  // all of batches can't meet the requirement, add a new batch
  if (!__seg) {
    auto __new_batch = this->add_BATCH();
    __seg            = __new_batch->allocate(size);
  }
  // if still fail
  if (!__seg) {
    logger_->error("新增的Batch也无法分配空间, 真奇怪...");
    return nullptr;
  }

  // will never reach this step.
  return nullptr;
}

int
mmgr::instbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->allocated_segments_.find(segment_id);
  if (__iter == this->allocated_segments_.end()) {
    logger_->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is instant segment
  if (typeid(instant_segment) != typeid(*__iter->second.get())) {
    logger_->error(
      "Segment_{}不是一个shm_kernel::memory_manager::instant_segment",
      segment_id);
    return -1;
  }
  // cast to instant segment
  auto __seg = std::dynamic_pointer_cast<instant_segment>(__iter->second);
  // free
  return this->instant_bin_->free(__seg);
}

int
mmgr::statbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->allocated_segments_.find(segment_id);
  if (__iter == this->allocated_segments_.end()) {
    logger_->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is static segment
  if (typeid(static_segment) != typeid(*__iter->second.get())) {
    logger_->error(
      "Segment_{}不是一个shm_kernel::memory_manager::static_segment",
      segment_id);
    return -1;
  }
  // cast to static segment
  auto __seg = std::dynamic_pointer_cast<static_segment>(__iter->second);
  // free
  return this->batches_[__seg->batch_id]->deallocate(__seg);
}

int
mmgr::cachbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->allocated_segments_.find(segment_id);
  if (__iter == this->allocated_segments_.end()) {
    logger_->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is cache segment
  if (typeid(static_segment) != typeid(*__iter->second.get())) {
    logger_->error(
      "Segment_{}不是一个shm_kernel::memory_manager::cache_segment",
      segment_id);
    return -1;
  }
  // cast to cache segment
  auto __seg = std::dynamic_pointer_cast<cache_segment>(__iter->second);
  // free
  return this->cache_bin_->free(__seg);
}

void
mmgr::set_logger(std::shared_ptr<spdlog::logger> logger)
{
  this->logger_ = logger;
}

std::string_view
mmgr::memmgr_name() const noexcept
{
  return this->config_.name;
}
}