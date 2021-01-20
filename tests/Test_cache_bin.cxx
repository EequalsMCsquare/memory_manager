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
  REQUIRE(bin.total_areas() == shm_kernel::memory_manager::BUFF_AREA_COUNT);
  REQUIRE(bin.free_areas() == shm_kernel::memory_manager::BUFF_AREA_COUNT);

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

    std::promise<std::shared_ptr<libmem::cache_segment>> __seg_promise;
    auto __seg_future = __seg_promise.get_future();

    auto __rv_future = bin.async_malloc(sizeof(double) * 100, __seg_promise);
    auto __seg       = __seg_future.get();

    REQUIRE(__seg->id_ == 0);
    REQUIRE(__seg->size_ == 800);

    auto seg = libmem::segmentdesc(*__seg.get());

    auto  shm      = shm_kernel::shared_memory::shm_handle(seg.shmhdl_name());
    char* __buffer = (char*)shm.map();
    REQUIRE(__buffer != nullptr);
    void*                    __data_area = __buffer + seg.addr_pshift;
    std::condition_variable* __condv =
      reinterpret_cast<std::condition_variable*>(__buffer + seg.condv_pshift);

    std::memcpy(__data_area, src_arr.data(), 800);
    std::cout << "Before notify_all()" << std::endl;
    __condv->notify_all();
    std::cout << "after notify_all()" << std::endl;

    std::this_thread::sleep_for(50ms);
    std::cout << "Good Morning" << std::endl;
    auto rv = __rv_future.get();
    REQUIRE(rv == 0);
  }
}

// SCENARIO("Make use of a cache bin", "[cache_bin]")
// {
//   GIVEN("a segment_counter, arena_name and cache bin")
//   {
//     std::atomic_size_t counter = 0;
//     std::string        arena_name{ "test_arena" };
//     libmem::cache_bin  bin(counter, arena_name, 1_KB);
//     REQUIRE(bin.area_size() == 1_KB);
//     REQUIRE(bin.total_areas() ==
//     shm_kernel::memory_manager::BUFF_AREA_COUNT); REQUIRE(bin.free_areas() ==
//     shm_kernel::memory_manager::BUFF_AREA_COUNT);

//     WHEN("Copy a <= 1_KB buffer to the cache bin")
//     {
//       double src_sum   = 0;
//       double local_sum = 0;
//       GIVEN("800KB array")
//       {
//         std::array<double, 100> src_arr;

//         THEN("Asign value and calculate sum")
//         {
//           uint32_t i;
//           for (i = 0; i < 100; i++) {
//             src_arr[i] = i;
//             src_sum += i;
//           }
//           INFO("Src Array Sum: " << src_sum);
//         }
//         GIVEN("a promise to receive cache_segment future")
//         {
//           std::promise<std::shared_ptr<libmem::cache_segment>> __seg_promise;
//           auto __seg_future = __seg_promise.get_future();
//           THEN("get the cache_segment")
//           {
//             auto __rv_future =
//               bin.async_malloc(sizeof(double) * 100, __seg_promise);
//             auto __seg = __seg_future.get();

//             REQUIRE(__seg->id_ == 0);
//             REQUIRE(__seg->size_ == 800);
//             AND_THEN("Parse it to segmentdesc")
//             {
//               auto seg = libmem::segmentdesc(*__seg.get());

//               AND_THEN("Attach to it")
//               {
//                 auto shm =
//                   shm_kernel::shared_memory::shm_handle(seg.shmhdl_name());
//                 char* __buffer = (char*)shm.map();
//                 REQUIRE(__buffer != nullptr);
//                 void* __data_area = __buffer + seg.addr_pshift;
//                 std::condition_variable* __condv =
//                   reinterpret_cast<std::condition_variable*>(__buffer +
//                                                              seg.condv_pshift);

//                 AND_THEN("Copy src_arr")
//                 {
//                   std::memcpy(__data_area, src_arr.data(), 800);
//                   AND_THEN("notify the conditoin variable")
//                   {
//                     __condv->notify_all();
//                   }
//                 }
//               }
//             }
//             AND_THEN("future rv should == 0")
//             {
//               REQUIRE(__rv_future.get() == 0);
//             }
//           }
//         }
//       }

//       WHEN("Retrieve <= 1_KB buffer from cache_bin")
//       {
//         GIVEN("a promise to receive cache_segment future")
//         {
//           std::promise<std::shared_ptr<libmem::cache_segment>> __seg_promise;
//           auto __seg_future = __seg_promise.get_future();

//           THEN("Get cache segment")
//           {
//             auto __rv_future = bin.async_retrieve(0, __seg_promise);
//             auto __seg       = __seg_future.get();

//             REQUIRE(__seg->id_ == 0);
//             REQUIRE(__seg->size_ == 800);

//             AND_THEN("Parse it to segmentdesc")
//             {
//               auto seg = libmem::segmentdesc(*__seg.get());

//               AND_THEN("Attacch to it")
//               {
//                 auto shm =
//                   shm_kernel::shared_memory::shm_handle(seg.shmhdl_name());
//                 char* __buffer = (char*)shm.map();
//                 REQUIRE(__buffer != nullptr);
//                 void* __data_area = __buffer + seg.addr_pshift;
//                 std::condition_variable* __condv =
//                   reinterpret_cast<std::condition_variable*>(__buffer +
//                                                              seg.condv_pshift);
//                 GIVEN("A 100 size double array")
//                 {
//                   std::array<double, 100> local_arr;
//                   AND_THEN("Copy data to local arr")
//                   {
//                     std::memcpy(local_arr.data(), __data_area, 800);
//                   }
//                   AND_THEN("Calculate local sum")
//                   {
//                     for (const auto& e : local_arr) {
//                       local_sum += e;
//                     }
//                   }
//                 }
//               }
//             }
//           }
//         }
//       }
//       THEN("Check Src Sum == Local Sum") { REQUIRE(local_sum == src_sum); }
//     }
//   }
// }
