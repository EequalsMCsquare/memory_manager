#include "except.hpp"
#include <fmt/format.h>

namespace shm_kernel::memory_manager {

MmgrExcept::MmgrExcept(const MmgrErrc ec)
{
  std::error_code __tmp_ec = ec;

  this->_M_What = fmt::format("({}) {}", __tmp_ec.value(), __tmp_ec.message());
}

MmgrExcept::MmgrExcept(const std::error_code ec)
{
  this->_M_What = fmt::format("({}) {}", ec.value(), ec.message());
}
const char*
MmgrExcept::what() const noexcept
{
  return this->_M_What.c_str();
}

}