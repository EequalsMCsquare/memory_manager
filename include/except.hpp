#pragma once

#include <stdexcept>

#include "ec.hpp"

namespace shm_kernel::memory_manager {
class MmgrExcept : std::exception
{

private:
  std::string _M_What;

public:
  MmgrExcept(const MmgrErrc ec);
  MmgrExcept(const std::error_code ec);

  const char* what() const noexcept final override;
};
}