#pragma once

#include <stdexcept>

#include "error_condition.hpp"

namespace shm_kernel::memory_manager {
class MmgrExcept : std::exception
{

private:
  std::error_code _M_Ec;

public:
  MmgrExcept(const MmgrErrc ec);
  MmgrExcept(const std::error_code ec);

  const char* what() const noexcept final override;
};
}