#include "memmgr.hpp"
#include <spdlog/spdlog.h>

namespace shm_kernel::memory_manager {

memmgr::memmgr(memmgr_config&& config)
  : config_(config)
{
  // if no logger provided, use the deafult one
  this->logger_ = spdlog::default_logger();
}
}