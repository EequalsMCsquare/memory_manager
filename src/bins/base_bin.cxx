#include "bins/base_bin.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace libmem {
base_bin::base_bin(std::atomic_size_t& segment_counter) noexcept
  : segment_counter_ref_(segment_counter)
{}

base_bin::base_bin(std::atomic_size_t&             segment_counter,
                   std::shared_ptr<spdlog::logger> logger) noexcept
  : base_bin(segment_counter)
{
  this->logger_ = logger;
  spdlog::set_default_logger(this->logger_);
}
} // namespace libmem
