#include "arena.hpp"
#include "bins/instant_bin.hpp"
#include "mem_literals.hpp"
#include "segment.hpp"
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace shm_kernel::memory_manager {

constexpr std::initializer_list<size_t> default_statbin_chunksz{
  512,   1_KB,  2_KB,  4_KB,  8_KB,  12_KB, 16_KB,
  24_KB, 32_KB, 48_KB, 64_KB, 80_KB, 96_KB, 128_KB
};
constexpr std::initializer_list<size_t> default_statbin_chunkcnt{
  64, 64, 64, 64, 64, 64, 64, 64, 50, 50, 50, 50, 32, 32
};

arena::arena(std::string name)
  : segment_counter_(0)
{
  this->name_ = std::move(name);
  this->init_cache_bin();
  this->init_instant_bin();
  this->add_batch();
}

arena::~arena()
{
  spdlog::trace("begin to clean arena_{}", this->name());

  // TODO:
  spdlog::trace("arena_{} clean up complete.", this->name());
}

void
arena::add_batch()
{
  spdlog::trace("add new batch. Current batch count: {}",
                this->batches_.size() + 1);
  this->batches_.push_back(std::make_unique<batch>(name_,
                                                   this->batches_.size(),
                                                   default_statbin_chunksz,
                                                   default_statbin_chunkcnt));
}

void
arena::init_instant_bin()
{
  spdlog::trace("initializing instant bin...");
  this->instant_bin_ =
    std::make_unique<instant_bin>(1, this->segment_counter_, this->name());
  spdlog::trace("instant bin initialized!");
}

void
arena::init_cache_bin()
{
  spdlog::trace("initializing cache bin...");

  // TODO:
  spdlog::trace("cache bin initialized!");
}

std::shared_ptr<base_segment>
arena::allocate(const size_t nbytes)
{
  spdlog::trace("allocating {} bytes segment.", nbytes);
  std::shared_ptr<base_segment> __seg;

  if (nbytes <= this->batches_.front()->min_chunksz()) {
    // cache bin allocate
    throw std::runtime_error("hasn't implement cache bin.");
    // return std::move(this->cache_bin_->malloc(nbytes));
  } else if (nbytes > this->batches_.front()->max_chunksz()) {
    // instant bin
    return std::move(this->instant_bin_->malloc(nbytes));
  } else {
    // static bin
    for (const auto& batch : this->batches_) {
      __seg = batch->allocate(nbytes);
      if (__seg != nullptr) {
        return std::move(__seg);
      }
    }
    // all of batch can't meet requirement, add a new batch
    this->add_batch();
    // use the new added batch to allocate
    __seg = this->batches_.back()->allocate(nbytes);
    // if __seg is still nullptr, throw
    if (__seg == nullptr) {
      throw std::runtime_error(
        fmt::format("fail to allocate {} bytes", nbytes));
    }
  }
  // will never reach next line
  return nullptr;
}

void
arena::deallocate(std::shared_ptr<base_segment> segment)
{
  spdlog::trace("deallocating segment.");
  if (segment->type_ == SEG_TYPE::cachbin_segment) {
    // cache bin deallocate
  } else if (segment->type_ == SEG_TYPE::instbin_segment) {
    // instant bin deallocate
  } else if (segment->type_ == SEG_TYPE::statbin_segment) {
    // static bin deallocate
  } else {
    throw std::runtime_error(
      fmt::format("fail to deallocate segment: {}", segment->id_));
  }
}

std::string_view
arena::name() noexcept
{
  return this->name_;
}

size_t
arena::batch_count() noexcept
{
  return this->batches_.size();
}
}
