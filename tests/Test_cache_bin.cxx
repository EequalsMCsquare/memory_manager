#include "bins/cache_bin.hpp"
#include "segment.hpp"
#include <array>
#include <future>
#define CATCH_CONFIG_MAIN
#include "mem_literals.hpp"
#include <catch2/catch.hpp>

namespace libmem = shm_kernel::memory_manager;

std::atomic_size_t counter = 1;
std::string        arena_name{ "test_arena" };

TEST_CASE("create cache_bin", "[create]")
{
  libmem::cache_bin bin(0, counter, arena_name, 1_KB);
  REQUIRE(bin.id() == 0);
  REQUIRE(bin.area_size() == 1_KB);
  REQUIRE(bin.total_areas() == shm_kernel::memory_manager::BUFF_AREA_COUNT);
  REQUIRE(bin.free_areas() == shm_kernel::memory_manager::BUFF_AREA_COUNT);
}

TEST_CASE("transfering data", "[malloc]")
{
  // create c++ style array
  std::array<double, 10> src_arr{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  // create cache bin
  libmem::cache_bin bin(0, counter, arena_name, 1_KB);
  // create a promse
  std::promise<std::shared_ptr<libmem::base_segment>> buffarea_descriptor;

  auto rv      = bin.async_malloc(sizeof(src_arr), buffarea_descriptor);
  auto segment = buffarea_descriptor.get_future().get();
  auto shm     = bin.get_shmhdl();
  auto addr    = shm->map()
}