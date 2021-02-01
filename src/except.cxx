#include "except.hpp"

namespace shm_kernel::memory_manager {

MmgrExcept::MmgrExcept(const MmgrErrc ec)
{
  this->_M_Ec = ec;
}

const char*
MmgrExcept::what() const noexcept
{
  return this->_M_Ec.message().data();
}

MmgrExcept::MmgrExcept(const std::error_code ec)
{
  this->_M_Ec = ec;
}

}