#pragma once
#include "base_bin.hpp"
#include <memory_resource>

namespace libmem {
class cache_bin : base_bin
{
private:
  std::pmr::unsynchronized_pool_resource pmr_pool_;

public:
};
}