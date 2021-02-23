#pragma once
#include "segment.hpp"
#include <atomic>
#include <cstddef>
#include <map>
#include <memory_resource>
#include <spdlog/spdlog.h>
#include <string>
#include <ipc/shmhdl.hpp>
#include <string_view>
#include <system_error>

namespace shm_kernel::memory_manager {



using shm      = ipc::shmhdl;
using buffer   = std::pair<void*, size_t>;
using shm_refc = std::pair<std::shared_ptr<shm>, size_t>;

class smgr
{
private:
  // used to store cache segment
  std::shared_ptr<spdlog::logger>              logger_;
  std::pmr::unsynchronized_pool_resource       pmr_pool_;
  std::map<std::string, shm_refc, std::less<>> attached_shm_;
  std::map<size_t, std::shared_ptr<segment_info>, std::less<>>
    attached_segment_;

public:
  const std::string name_;

  smgr(std::string_view name,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());
  smgr(std::string&& name,
       std::shared_ptr<spdlog::logger> = spdlog::default_logger());

  std::shared_ptr<segment_info> register_segment(const segment_info* segment,
                                                 std::error_code& ec) noexcept;

  void unregister_segment(const size_t     segment_id,
                          std::error_code& ec) noexcept;
  /**
   * @brief 通过segment_info来获取segment在当前进程的buffer ptr
   *
   * @return std::pair<void*, size_t>
   */
  buffer bufferize(std::shared_ptr<segment_info>, std::error_code& ec) noexcept;
  buffer bufferize(const size_t segment_id, std::error_code& ec) noexcept;
};

}
