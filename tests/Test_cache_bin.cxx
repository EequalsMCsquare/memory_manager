#include "bins/cache_bin.hpp"
#include "segment.hpp"
#include <array>
#include <condition_variable>
#include <cstring>
#include <future>
#include <shm_kernel/shared_memory.hpp>
#include <thread>
#define CATCH_CONFIG_MAIN
#include "mem_literals.hpp"
#include <catch2/catch.hpp>
#include <chrono>

namespace libmem = shm_kernel::memory_manager;
using namespace std::chrono_literals;

TEST_CASE("Create a cache bin", "[cache_bin]")
{
  std::atomic_size_t counter = 0;
  std::string        arena_name{ "test_arena" };
  libmem::cache_bin  bin(counter, arena_name, 1_KB);
  REQUIRE(bin.area_size() == 1_KB);

  SECTION("Copy arr to cache bin")
  {

    double src_sum   = 0;
    double local_sum = 0;

    std::array<double, 100> src_arr;
    uint32_t                i;
    for (i = 0; i < 100; i++) {
      src_arr[i] = i;
      src_sum += i;
    }
    INFO("Src Array Sum: " << src_sum);

    auto seg_id = bin.store(src_arr.data(), sizeof(src_arr));
    REQUIRE(seg_id >= 0);
    REQUIRE(bin.segment_count() == 1);
    SECTION("Retrive the data")
    {
      size_t  __seg_size;
      double* __local_buffer =
        static_cast<double*>(bin.retrieve(0, __seg_size));
      REQUIRE(__local_buffer != nullptr);
      REQUIRE(__seg_size == 800);
      uint32_t i;
      for (i = 0; i < 100; i++) {
        local_sum += __local_buffer[i];
      }
      REQUIRE(local_sum == src_sum);
    }

    SECTION("free the segment")
    {
      auto rv = bin.free(0);
      REQUIRE(rv == 0);
      REQUIRE(bin.segment_count() == 0);
    }
  }
}
