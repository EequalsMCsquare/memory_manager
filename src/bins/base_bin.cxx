#include "bins/base_bin.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace libmem {
base_bin::base_bin(const size_t        id,
                   std::atomic_size_t& segment_counter) noexcept
  : segment_counter_ref_(segment_counter)
  , id_(id)
{}

base_bin::base_bin(const size_t                    id,
                   std::atomic_size_t&             segment_counter,
                   std::shared_ptr<spdlog::logger> logger) noexcept
  : base_bin(id, segment_counter)
{
  this->logger_ = logger;
  spdlog::set_default_logger(this->logger_);
}

const size_t
base_bin::id() const noexcept
{
  return this->id_;
}
} // namespace libmem
