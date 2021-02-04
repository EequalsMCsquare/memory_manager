#pragma once
#include "segment.hpp"
#include <map>
#include <memory_resource>
#include <shared_memory.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
namespace shm_kernel::memory_manager {

using buffer = std::pair<void*, size_t>;

class smgr
{
private:
  using shm = shm_kernel::shared_memory::shm_handle;

  // used to store cache segment
  std::shared_ptr<spdlog::logger>                          logger_;
  std::pmr::unsynchronized_pool_resource                   pmr_pool_;
  std::map<std::string, std::shared_ptr<shm>, std::less<>> attached_shm_;
  std::map<size_t, std::shared_ptr<segment_info>, std::less<>>
    attached_segment_;

public:
  const std::string name_;

  smgr(std::string_view name,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());
  smgr(std::string&& name,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  void register_segment(std::shared_ptr<segment_info>,
                        std::error_code& ec) noexcept;
  /**
   * @brief 通过segment_info来获取segment在当前进程的buffer ptr
   *
   * @return std::pair<void*, size_t>
   */
  buffer bufferize(std::shared_ptr<segment_info>, std::error_code& ec);
};

}
