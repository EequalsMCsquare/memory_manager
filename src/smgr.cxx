#include "smgr.hpp"
#include "segment.hpp"

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

}