#include "mmgr.hpp"
#include "bins/cache_bin.hpp"
#include "bins/instant_bin.hpp"
#include "segment.hpp"
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

namespace shm_kernel::memory_manager {

void
mmgr::PRE_CHECK() const
{
  if (this->batch_bin_count_.size() != this->batch_bin_size_.size()) {
    _M_mmgr_logger->critical("Batch Bin Size的长度与Batch Bin Count的不一致!");
    throw std::runtime_error(
      "batch_bin_size.size() != batch_bin_count.size()!");
  }
}

mmgr::mmgr(std::string&&                   name,
           std::vector<size_t>&&           batch_bin_size,
           std::vector<size_t>&&           batch_bin_count,
           std::shared_ptr<spdlog::logger> logger)
  : name_(name)
  , batch_bin_count_(batch_bin_count)
  , batch_bin_size_(batch_bin_size)
  , _M_mmgr_logger(logger)
{
  _M_mmgr_logger->trace("正在初始化Memory Manager...");
  this->PRE_CHECK();
  this->init_INSTANT_BIN();
  this->init_CACHE_BIN();
  this->add_BATCH();
  _M_mmgr_logger->trace("Memory Manager 初始化完毕!");
}

mmgr::~mmgr()
{
  _M_mmgr_logger->trace("正在清理shm_kernel::memory_manager::mmgr...");

  _M_mmgr_logger->trace("shm_kernel::memory_manager::mmgr清理完毕!");
}

void
mmgr::init_INSTANT_BIN()
{
  this->instant_bin_ = std::make_shared<instant_bin>(
    this->segment_counter_, this->name(), this->_M_mmgr_logger);
}

void
mmgr::init_CACHE_BIN()
{
  this->cache_bin_ =
    std::make_shared<cache_bin>(segment_counter_, name(), this->_M_mmgr_logger);
}

std::shared_ptr<batch>
mmgr::add_BATCH()
{
  std::lock_guard<std::mutex> GG(this->mtx_);
  this->batches_.push_back(std::make_shared<batch>(this->name(),
                                                   batches_.size(),
                                                   segment_counter_,
                                                   batch_bin_size_,
                                                   batch_bin_count_,
                                                   this->_M_mmgr_logger));
  return this->batches_.back();
}

std::shared_ptr<cache_segment>
mmgr::cachbin_STORE(const void* buffer, const size_t size) noexcept
{
  if (buffer == nullptr) {
    _M_mmgr_logger->error("Buffer 不能为空指针!");
    return nullptr;
  }
  auto __seg       = this->cache_bin_->store(buffer, size);
  __seg->mmgr_name = this->name();
  auto __insert_rv =
    this->segment_table_.insert(std::make_pair(__seg->id, __seg));
  if (!__insert_rv.second) {
    _M_mmgr_logger->error("无法将Segment添加进Table!");
    this->cache_bin_->free(__seg);
    return nullptr;
  }
  return __seg;
}

void*
mmgr::cachbin_RETRIEVE(const size_t segment_id) noexcept
{
  auto __iter = this->segment_table_.find(segment_id);
  if (__iter == this->segment_table_.end()) {
    _M_mmgr_logger->error("没有找到Segment {}", segment_id);
    return nullptr;
  }
  auto __buff = this->cache_bin_->retrieve(segment_id);
  if (__buff == nullptr) {
    _M_mmgr_logger->error("segment {} 不是一个Cache Segment!", segment_id);
    return nullptr;
  }
  return __buff;
}

std::shared_ptr<instant_segment>
mmgr::instbin_ALLOC(const size_t size) noexcept
{
  auto __seg       = this->instant_bin_->malloc(size);
  __seg->mmgr_name = this->name();
  auto __iter_rv =
    this->segment_table_.insert(std::make_pair(__seg->id, __seg));
  if (!__iter_rv.second) {
    _M_mmgr_logger->error("无法将Segment添加进Table!");
    this->instant_bin_->free(__seg);
    return nullptr;
  }
  return __seg;
}

std::shared_ptr<static_segment>
mmgr::statbin_ALLOC(const size_t size) noexcept
{
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
    // if still fail
    if (!__seg) {
      _M_mmgr_logger->error("新增的Batch也无法分配空间, 真奇怪...");
      return nullptr;
    }
  }
  __seg->mmgr_name = this->name();
  auto __insert_rv =
    this->segment_table_.insert(std::make_pair(__seg->id, __seg));
  if (!__insert_rv.second) {
    this->batches_[__seg->batch_id]->deallocate(__seg);
    _M_mmgr_logger->error("无法将Segment添加进Table.");
    return nullptr;
  }
  return __seg;

  // will never reach this step.
  return nullptr;
}

int
mmgr::instbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->segment_table_.find(segment_id);
  if (__iter == this->segment_table_.end()) {
    _M_mmgr_logger->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is instant segment
  if (__iter->second->type != SEG_TYPE::instbin_segment) {
    _M_mmgr_logger->error(
      "Segment_{}不是一个shm_kernel::memory_manager::instant_segment",
      segment_id);
    return -1;
  }
  // cast to instant segment
  auto __seg = std::dynamic_pointer_cast<instant_segment>(__iter->second);
  // free
  int rv = this->instant_bin_->free(__seg);
  if (rv == 0) {
    this->segment_table_.erase(__iter);
    return 0;
  }
  _M_mmgr_logger->error("Segment dealloc失败!");
  return -1;
}

int
mmgr::statbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->segment_table_.find(segment_id);
  if (__iter == this->segment_table_.end()) {
    _M_mmgr_logger->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is static segment
  if (__iter->second->type != SEG_TYPE::statbin_segment) {
    _M_mmgr_logger->error(
      "Segment_{}不是一个shm_kernel::memory_manager::static_segment",
      segment_id);
    return -1;
  }
  // cast to static segment
  auto __seg = std::dynamic_pointer_cast<static_segment>(__iter->second);
  // free
  int rv = this->batches_[__seg->batch_id]->deallocate(__seg);
  if (rv == 0) {
    this->segment_table_.erase(__iter);
    return 0;
  }
  // fail
  _M_mmgr_logger->error("Segment dealloc失败");
  return -1;
}

int
mmgr::cachbin_DEALLOC(const size_t segment_id) noexcept
{
  auto __iter = this->segment_table_.find(segment_id);
  if (__iter == this->segment_table_.end()) {
    _M_mmgr_logger->error("没有找到Segment");
    return -1;
  }
  // if found  cehck if segment is cache segment
  if (__iter->second->type != SEG_TYPE::cachbin_segment) {
    _M_mmgr_logger->error(
      "Segment_{}不是一个shm_kernel::memory_manager::cache_segment",
      segment_id);
    return -1;
  }
  // cast to cache segment
  auto __seg = std::dynamic_pointer_cast<cache_segment>(__iter->second);
  // free
  int rv = this->cache_bin_->free(__seg);
  if (rv == 0) {
    this->segment_table_.erase(__iter);
    return 0;
  }
  // fail
  _M_mmgr_logger->error("Segment dealloc失败!");
  return -1;
}

void
mmgr::set_logger(std::shared_ptr<spdlog::logger> logger)
{
  this->_M_mmgr_logger = logger;
}

std::shared_ptr<base_segment>
mmgr::get_segment(const size_t segment_id) noexcept
{
  auto __iter = this->segment_table_.find(segment_id);
  if (__iter == this->segment_table_.end()) {
    return {};
  }
  return __iter->second;
}

std::string_view
mmgr::name() const noexcept
{
  return this->name_;
}
size_t
mmgr::segment_count() const noexcept
{
  return this->segment_table_.size();
}

const std::vector<size_t>&
mmgr::batch_bin_size() const noexcept
{
  return this->batch_bin_size_;
}
const std::vector<size_t>&
mmgr::batch_bin_count() const noexcept
{
  return this->batch_bin_count_;
}
}