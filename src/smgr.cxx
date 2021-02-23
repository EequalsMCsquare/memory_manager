#include "smgr.hpp"
#include "ec.hpp"
#include "segment.hpp"

#include <atomic>
#include <utility>
namespace shm_kernel::memory_manager {
smgr::smgr(std::string_view name, std::shared_ptr<spdlog::logger> logger)
  : name_(name)
  , logger_(logger)
{}

smgr::smgr(std::string&& name, std::shared_ptr<spdlog::logger> logger)
  : name_(std::forward<std::string&>(name))
  , logger_(logger)
{
  logger_->trace("正在初始化Segment Manager...");
  // TODO
  logger_->trace("Segment Manager初始化完毕!");
}

std::shared_ptr<segment_info>
smgr::register_segment(const segment_info* segment,
                       std::error_code&    ec) noexcept
{
  ec.clear();
  // check if segment already exist
  auto __seg_iter = this->attached_segment_.find(segment->id_);
  if (__seg_iter != this->attached_segment_.end()) {
    this->logger_->error("segment already exist!");
    ec = MmgrErrc::SegmentExist;
    return nullptr;
  }

  if (segment->type() == SEG_TYPE::CACHE_SEGMENT) {
    auto  __seg          = std::make_shared<segment_info>(*segment);
    void* __cache_buffer = this->pmr_pool_.allocate(segment->size());
    if (__cache_buffer == nullptr) {
      ec = MmgrErrc::NoMemory;
      return nullptr;
    }
    __seg->set_ptr(__cache_buffer);
    return __seg;
  } else {
    // insert segment_info
    auto insert_seg_rv = this->attached_segment_.insert(
      { segment->id_, std::make_shared<segment_info>(*segment) });
    auto __seg = insert_seg_rv.first->second;

    auto __shm_name = segment->shm_name();
    auto __shm_iter = this->attached_shm_.find(__shm_name);
    if (__shm_iter == attached_shm_.end()) {
      // attach
      std::shared_ptr<shm> __shm;
      try {
        __shm = std::make_shared<shm>(__shm_name);
      } catch (...) {
        ec = MmgrErrc::UnableToAttachShm;
        return nullptr;
      }
      auto __insert_shm_rv =
        this->attached_shm_.insert({ __shm_name, { __shm, 1 } });
      // set current process addr
      
      __seg->set_ptr(static_cast<char*>(__shm->map(ec)) + __seg->addr_pshift_);
      return __seg;
    } else {
      // if shm object found, increase the local ref_count

      auto* __buffer = static_cast<char*>(__shm_iter->second.first->map(ec));
      if (__buffer == nullptr) {
        ec = MmgrErrc::NullptrBuffer;
        return nullptr;
      }
      __shm_iter->second.second += 1;
      __seg->set_ptr(__buffer + __seg->addr_pshift_);
      return __seg;
    }
  }
  return nullptr;
}

void
smgr::unregister_segment(const size_t segment_id, std::error_code& ec) noexcept
{
  ec.clear();
  // check if segment exist
  auto __seg_iter = this->attached_segment_.find(segment_id);
  if (__seg_iter == this->attached_segment_.end()) {
    ec = MmgrErrc::SegmentNotFound;
    return;
  }
  auto __seg = __seg_iter->second;
  //  unregister for a cache_segment
  if (__seg->type() == SEG_TYPE::CACHE_SEGMENT) {
    this->pmr_pool_.deallocate(__seg->local_buffer_, __seg->size());
    this->attached_segment_.erase(__seg_iter);
    return;
  }
  // unregister for a shm_segment
  auto __shm_refc_iter = this->attached_shm_.find(__seg->shm_name());
  if (__shm_refc_iter == this->attached_shm_.end()) {
    auto& __shm_refc = __shm_refc_iter->second.second;
    __shm_refc--;
    if (__shm_refc == 0) {
      this->attached_shm_.erase(__shm_refc_iter);
    }
    // erase segment
    this->attached_segment_.erase(__seg_iter);
  }
}

buffer
smgr::bufferize(std::shared_ptr<segment_info> seg, std::error_code& ec) noexcept
{
  ec.clear();
  auto __seg = this->attached_segment_[seg->id()];
  if (__seg) {
    return { __seg->local_buffer_, __seg->size() };
  }
  ec = MmgrErrc::NullptrBuffer;
  return { nullptr, 0 };
}

buffer
smgr::bufferize(const size_t segment_id, std::error_code& ec) noexcept
{
  ec.clear();
  auto __seg = this->attached_segment_[segment_id];
  if (__seg) {
    return { __seg->local_buffer_, __seg->size() };
  }
  ec = MmgrErrc::NullptrBuffer;
  return { nullptr, 0 };
}
}