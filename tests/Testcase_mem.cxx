#include "bins/cache_bin.hpp"
#include "mmgr.hpp"
#include "segment.hpp"
#include "smgr.hpp"
#include <array>
#define CATCH_CONFIG_MAIN
#include "batch.hpp"
#include "mem_literals.hpp"
#include <catch2/catch.hpp>
#include <chrono>

namespace libmem = shm_kernel::memory_manager;
using namespace std::chrono_literals;

SCENARIO("Make use of an existing batch", "[batch]")
{
  GIVEN("A 56MB batch and a segment_counter")
  {
    std::error_code    ec;
    std::atomic_size_t segment_counter{ 0 };
    libmem::batch      batch(
      "test_arena", 1, segment_counter, 4_KB, 128_KB, 8_KB, 64);
    INFO("batch total bytes: " << (batch.total_bytes() / 1024 / 1024) << " MB");
    REQUIRE(batch.min_chunksz() == 4_KB);
    REQUIRE(batch.max_chunksz() <= 256_KB);
    REQUIRE(batch.id() == 1);
    REQUIRE(batch.mmgr_name().compare("test_arena") == 0);

    GIVEN("A 16KB segment")
    {
      auto seg = batch.allocate(16_KB, ec);
      REQUIRE(seg->batch_id == 1);
      REQUIRE(seg->id == 0);
      REQUIRE(batch.mmgr_name().compare("test_arena") == 0);
      REQUIRE(seg->size == 16_KB);
      REQUIRE(seg->bin_id == 0);

      THEN("the segment counter should increase")
      {
        REQUIRE(segment_counter == 1);
      }
      WHEN("The segment is deallocated")
      {
        auto rv = batch.deallocate(seg, ec);
        THEN("There should be no error") { REQUIRE(rv == 0); }
      }

      GIVEN("A 20KB segment")
      {
        auto seg = batch.allocate(20_KB, ec);
        REQUIRE(seg->batch_id == 1);
        REQUIRE(seg->mmgr_name.compare("test_arena") == 0);
        REQUIRE(seg->id == 1);
        REQUIRE(seg->size == 20_KB);
        REQUIRE(seg->bin_id == 2);

        THEN("the segment counter should increase")
        {
          REQUIRE(segment_counter == 2);
        }
        WHEN("The segment is deallocated")
        {
          auto rv = batch.deallocate(seg, ec);
          THEN("There should be no error") { REQUIRE(rv == 0); }
        }

        GIVEN("A 28_KB segment")
        {
          auto seg = batch.allocate(28_KB, ec);
          REQUIRE(seg->batch_id == 1);
          REQUIRE(seg->mmgr_name.compare("test_arena") == 0);
          REQUIRE(seg->id == 2);
          REQUIRE(seg->size == 28_KB);
          REQUIRE(seg->bin_id == 3);

          THEN("the segment counter should increase")
          {
            REQUIRE(segment_counter == 3);
          }
          WHEN("The segment is deallocated")
          {
            auto rv = batch.deallocate(seg, ec);
            THEN("There should be no error") { REQUIRE(rv == 0); }
          }

          GIVEN("A 32_KB segment")
          {
            auto seg = batch.allocate(32_KB, ec);
            REQUIRE(seg->batch_id == 1);
            REQUIRE(seg->mmgr_name.compare("test_arena") == 0);
            REQUIRE(seg->id == 3);
            REQUIRE(seg->size == 32_KB);
            REQUIRE(seg->bin_id == 0);

            THEN("the segment counter should increase")
            {
              REQUIRE(segment_counter == 4);
            }
            WHEN("The segment is deallocated")
            {
              auto rv = batch.deallocate(seg, ec);
              THEN("There should be no error") { REQUIRE(rv == 0); }
            }
          }
        }
      }
    }

    WHEN("Allocate size is > than max bin's chunk size * 8")
    {
      batch.allocate(9 * batch.max_chunksz(), ec);
      REQUIRE(ec);
      batch.allocate(16 * batch.max_chunksz(), ec);
      REQUIRE(ec);
      batch.allocate(32 * batch.max_chunksz(), ec);
      REQUIRE(ec);
      batch.allocate(32_MB, ec);
      REQUIRE(ec);
    }

    GIVEN("Give an ivalid batch_id segment")
    {
      auto seg      = std::make_shared<libmem::static_segment>();
      seg->batch_id = 100;
      WHEN("deallocate a invalid segment")
      {
        auto rv = batch.deallocate(seg, ec);

        THEN("Error should happen") { REQUIRE(rv == -1); }
      }
    }
  }
}

SCENARIO("Store buffer in a cache bin", "[cache_bin]")
{
  GIVEN("A segment_counter, memmgr_name")
  {
    std::error_code    ec;
    std::atomic_size_t segment_counter = 0;
    std::string        memmgr_name{ "Test_Arena" };

    THEN("Create a cache_bin")
    {
      libmem::cache_bin bin(segment_counter, memmgr_name);

      GIVEN("A 100 size double array")
      {
        double                  src_sum   = 0;
        double                  local_sum = 0;
        std::array<double, 100> src_arr;
        THEN("Assign value")
        {
          uint32_t i;
          for (i = 0; i < 100; i++) {
            src_arr[i] = i;
            src_sum += i;
          }
          INFO("Src Array Sum: " << src_sum);

          WHEN("Store buffer into cache bin")
          {
            auto seg = bin.store(src_arr.data(), sizeof(src_arr), ec);
            REQUIRE(seg != nullptr);
            REQUIRE(bin.segment_count() == 1);
            REQUIRE(seg->size == sizeof(src_arr));

            AND_WHEN("Retrive the data")
            {
              double* __local_buffer = (double*)bin.retrieve(seg->id, ec);
              REQUIRE(__local_buffer != nullptr);
              uint32_t i;
              for (i = 0; i < 100; i++) {
                local_sum += __local_buffer[i];
              }
              REQUIRE(local_sum == src_sum);
            }
            AND_WHEN("Free the segment")
            {
              auto rv = bin.free(seg, ec);
              REQUIRE(rv == 0);
              REQUIRE(bin.segment_count() == 0);
            }
          }
        }
      }
    }
  }
}

TEST_CASE("create instant bin", "[instant_bin]")
{
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");
  REQUIRE(bin.shmhdl_count() == 0);
}

TEST_CASE("instant bin malloc", "[instant_bin]")
{
  std::error_code     ec;
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");
  REQUIRE(bin.shmhdl_count() == 0);

  auto seg1 = bin.malloc(4_MB, ec);
  REQUIRE(seg1 != nullptr);
  REQUIRE(bin.shmhdl_count() == 1);
  auto seg1_shmhdl = bin.get_shmhdl(seg1->id, ec);
  REQUIRE(seg1_shmhdl.use_count() == 2);
  REQUIRE(seg1_shmhdl->ref_count() == 1);
  REQUIRE(seg1_shmhdl->nbytes() >= 4_MB);

  auto seg2 = bin.malloc(16_MB, ec);
  REQUIRE(seg2 != nullptr);
  REQUIRE(bin.shmhdl_count() == 2);
  auto seg2_shmhdl = bin.get_shmhdl(seg2->id, ec);
  REQUIRE(seg2_shmhdl.use_count() == 2);
  REQUIRE(seg2_shmhdl->ref_count() == 1);
  REQUIRE(seg2_shmhdl->nbytes() >= 16_MB);

  auto seg3 = bin.malloc(128_MB, ec);
  REQUIRE(seg3 != nullptr);
  REQUIRE(bin.shmhdl_count() == 3);
  auto seg3_shmhdl = bin.get_shmhdl(seg3->id, ec);
  REQUIRE(seg3_shmhdl.use_count() == 2);
  REQUIRE(seg3_shmhdl->ref_count() == 1);
  REQUIRE(seg3_shmhdl->nbytes() >= 128_MB);
}

TEST_CASE("instant bin free", "[instant_bin]")
{
  std::error_code     ec;
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");

  auto seg1 = bin.malloc(4_MB, ec);
  REQUIRE(seg1 != nullptr);
  {
    auto seg1_shmhdl = bin.get_shmhdl(seg1->id, ec);
    REQUIRE(seg1_shmhdl->nbytes() >= 4_MB);
    REQUIRE(seg1_shmhdl->ref_count() == 1);
    REQUIRE(bin.size() == 1);
  }

  bin.free(seg1, ec);
  REQUIRE(bin.shmhdl_count() == 0);

  auto seg2 = bin.malloc(128_MB, ec);
  REQUIRE(seg2 != nullptr);
  {
    auto seg2_shmhdl = bin.get_shmhdl(seg2->id, ec);
    REQUIRE(seg2_shmhdl->nbytes() >= 128_MB);
    REQUIRE(seg2_shmhdl->ref_count() == 1);
    REQUIRE(bin.size() == 1);
  }

  bin.free(seg2, ec);
  REQUIRE(bin.shmhdl_count() == 0);
}

TEST_CASE("create static_bin", "[static_bin]")
{
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 200, 0);
  REQUIRE(bin.base_pshift() == 0);
  REQUIRE(bin.chunk_count() == 200);
  REQUIRE(bin.chunk_left() == 200);
  REQUIRE(bin.chunk_size() == 32);
}

TEST_CASE("when create static bin with unaligned chunk size, thorw",
          "[static_bin]")
{
  std::atomic_size_t counter = 1;
  REQUIRE_THROWS(libmem::static_bin(0, counter, 31, 200, 0));
  REQUIRE_THROWS(libmem::static_bin(0, counter, 47, 200, 0));
  REQUIRE_THROWS(libmem::static_bin(0, counter, 127, 200, 0));
  REQUIRE_NOTHROW(libmem::static_bin(0, counter, 128, 200, 0));
  REQUIRE_NOTHROW(libmem::static_bin(0, counter, 1024, 200, 0));
  REQUIRE_THROWS(libmem::static_bin(0, counter, 1024, 200, 1111));
  REQUIRE_THROWS(libmem::static_bin(0, counter, 1024, 200, 3333));
}

TEST_CASE("allocate with successful return", "[static_bin]")
{
  std::error_code    ec;
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 200, 0);
  auto               seg1 = bin.malloc(128, ec);
  REQUIRE(seg1);
  REQUIRE(seg1->addr_pshift == 0);
  REQUIRE(seg1->size == 128);

  auto seg2 = bin.malloc(128, ec);
  REQUIRE(seg2);
  REQUIRE(seg2->addr_pshift == 128);
  REQUIRE(seg2->size == 128);

  auto seg3 = bin.malloc(256, ec);
  REQUIRE(seg3);
  REQUIRE(seg3->addr_pshift == 256);
  REQUIRE(seg3->size == 256);

  auto seg4 = bin.malloc(512, ec);
  REQUIRE(seg4);
  REQUIRE(seg4->addr_pshift == 512);
  REQUIRE(seg4->size == 512);
}

TEST_CASE("allocate with error return", "[static_bin]")
{
  std::error_code    ec;
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 100, 0);
  // successful malloc
  auto seg1 = bin.malloc(512, ec);
  REQUIRE(seg1);

  // insufficient memory return -1
  auto seg2 = bin.malloc(32 * 101, ec);
  REQUIRE_FALSE(seg2);
}

TEST_CASE("free allocated buffer", "[static_bin]")
{
  std::error_code    ec;
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 200, 0);

  constexpr size_t ALLOC_SIZE = 256;

  // malloc
  auto seg1 = bin.malloc(ALLOC_SIZE, ec);
  REQUIRE(seg1);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  auto seg2 = bin.malloc(ALLOC_SIZE, ec);
  REQUIRE(seg2);
  REQUIRE(bin.chunk_left() == 200 - 2 * (ALLOC_SIZE / bin.chunk_size()));

  // free
  bin.free(seg1, ec);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 0 - 64 bytes
  auto seg3 = bin.malloc(64, ec);
  REQUIRE(seg3);
  REQUIRE(bin.chunk_left() == 200 - 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 64 - 128 bytes
  auto seg4 = bin.malloc(64, ec);
  REQUIRE(seg4);
  REQUIRE(bin.chunk_left() == 200 - 2 * 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 128 - 192 bytes
  auto seg5 = bin.malloc(64, ec);
  REQUIRE(seg5);
  REQUIRE(bin.chunk_left() == 200 - 2 * 3 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 192 - 256 bytes
  auto seg6 = bin.malloc(64, ec);
  REQUIRE(seg6);
  REQUIRE(bin.chunk_left() == 200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 512 - 768
  auto seg7 = bin.malloc(256, ec);
  REQUIRE(seg7);
  REQUIRE(bin.chunk_left() ==
          200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()) * 2);
}

TEST_CASE("ilegal range segment free error", "[static_bin]")
{
  std::error_code    ec;
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 10, 0);
  auto               __seg = std::make_shared<libmem::static_segment>();

  // ilegal ptr
  __seg->addr_pshift = 400;
  __seg->size        = 0;
  auto rv1           = bin.free(__seg, ec);
  REQUIRE(rv1 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal ptr
  __seg->addr_pshift = 800;
  __seg->size        = 0;
  auto rv2           = bin.free(__seg, ec);
  REQUIRE(rv2 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  __seg->addr_pshift = 0;
  __seg->size        = 800;
  auto rv3           = bin.free(__seg, ec);
  REQUIRE(rv3 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  __seg->addr_pshift = 0;
  __seg->size        = 400;
  auto rv4           = bin.free(__seg, ec);
  REQUIRE(rv4 == -1);
  REQUIRE(bin.chunk_left() == 10);
}

TEST_CASE("double free error", "[static_bin]")
{
  std::error_code    ec;
  std::atomic_size_t counter = 1;
  libmem::static_bin bin(0, counter, 32, 10, 0);

  auto __seg = std::make_shared<libmem::static_segment>();
  // double free
  __seg->addr_pshift = 0;
  __seg->size        = 128;
  auto rv1           = bin.free(__seg, ec);
  REQUIRE(rv1 == -1);
  REQUIRE(bin.chunk_left() == 10);

  auto seg = bin.malloc(128, ec);
  REQUIRE(seg);
  REQUIRE(seg->addr_pshift == 0);
  REQUIRE(bin.chunk_left() == 6);

  seg->size = 256;
  auto rv2  = bin.free(seg, ec);
  REQUIRE(rv2 == -1);
  REQUIRE(bin.chunk_left() == 6);
}

SCENARIO("allocate with mmgr", "[mmgr]")
{
  std::error_code ec;
  GIVEN("A mmgr")
  {
    libmem::mmgr pool("test", { 128 }, { 100 });
    WHEN("store a long with cache bin")
    {
      long num   = 100;
      auto __seg = pool.CACHE_STORE(&num, 8, ec);
      REQUIRE(__seg != nullptr);
      REQUIRE(__seg->id == 0);
      REQUIRE(__seg->size == 8);
      REQUIRE(__seg->mmgr_name.compare("test") == 0);
      THEN("segment_table should change")
      {
        REQUIRE(pool.segment_count() == 1);
      }
      AND_WHEN("retrive the segment")
      {
        long* __buff = static_cast<long*>(pool.CACHE_RETRIEVE(__seg->id, ec));
        REQUIRE(__buff != nullptr);
        REQUIRE(*__buff == num);
      }
      AND_WHEN("dealloc the segment")
      {
        pool.CACHE_DEALLOC(__seg->id, ec);
        THEN("segment count should be 0")
        {
          REQUIRE(pool.segment_count() == 0);
        }
      }
      AND_WHEN("set a segment")
      {
        GIVEN("a new number")
        {
          long new_num = 200;
          THEN("modify the value")
          {
            int rv = pool.CACHE_SET(__seg->id, &new_num, sizeof(long), ec);
            REQUIRE(rv == 0);
            AND_THEN("retrive the new data")
            {
              long* __new_buff =
                static_cast<long*>(pool.CACHE_RETRIEVE(__seg->id, ec));
              REQUIRE(__new_buff != nullptr);
              REQUIRE(*__new_buff == new_num);
            }
          }
        }
      }
    }
    WHEN("store long with cache bin using callback function")
    {
      double num   = 3.14;
      auto   __seg = pool.CACHE_STORE(
        sizeof(double),
        [num](void* buffer) { *(static_cast<double*>(buffer)) = num; },
        ec);
      REQUIRE(__seg != nullptr);
      REQUIRE(__seg->id == 0);
      REQUIRE(__seg->size == 8);
      REQUIRE(__seg->mmgr_name.compare("test") == 0);
      THEN("segment_table should change")
      {
        REQUIRE(pool.segment_count() == 1);
      }
      AND_WHEN("retrive the segment")
      {
        double* __buff =
          static_cast<double*>(pool.CACHE_RETRIEVE(__seg->id, ec));
        REQUIRE(__buff != nullptr);
        REQUIRE(*__buff == num);
      }
      AND_WHEN("dealloc the segment")
      {
        pool.CACHE_DEALLOC(__seg->id, ec);
        THEN("segment count should be 0")
        {
          REQUIRE(pool.segment_count() == 0);
        }
      }
      AND_WHEN("set a segment with callback")
      {
        GIVEN("a new number")
        {
          float new_num = 6.2831852;
          THEN("modify the value")
          {
            int rv = pool.CACHE_SET(
              __seg->id,
              sizeof(float),
              [new_num](void* buffer) {
                *(static_cast<float*>(buffer)) = new_num;
              },
              ec);
            REQUIRE(rv == 0);
            AND_THEN("retrive the new data")
            {
              float* __new_buff =
                static_cast<float*>(pool.CACHE_RETRIEVE(__seg->id, ec));
              REQUIRE(__new_buff != nullptr);
              REQUIRE(*__new_buff == new_num);
            }
          }
        }
      }
    }

    WHEN("alloc with static bin")
    {
      auto __seg = pool.STATIC_ALLOC(128, ec);
      REQUIRE(__seg != nullptr);
      REQUIRE(__seg->batch_id == 0);
      REQUIRE(__seg->bin_id == 0);
      REQUIRE(__seg->id == 0);
      REQUIRE(__seg->mmgr_name.compare("test") == 0);
      REQUIRE(pool.segment_count() == 1);

      WHEN("dealloc the segment")
      {
        pool.STATIC_DEALLOC(__seg->id, ec);
        THEN("segment count should be 0")
        {
          REQUIRE(pool.segment_count() == 0);
        }
      }
    }

    WHEN("alloc with instant bin")
    {
      auto __seg = pool.INSTANT_ALLOC(32_MB, ec);
      REQUIRE(__seg->type == libmem::SEG_TYPE::INSTANT_SEGMENT);
      REQUIRE(__seg->mmgr_name.compare("test") == 0);
      REQUIRE(__seg->id == 0);
      REQUIRE(__seg->size >= 32_MB);

      WHEN("dealloc the segment")
      {
        pool.INSTANT_DEALLOC(__seg->id, ec);
        THEN("segment count should be 0")
        {
          REQUIRE(pool.segment_count() == 0);
        }
      }
    }
  }
}

TEST_CASE("smgr test", "[smgr]")
{
  std::error_code ec;
  std::string     mmgr_name = "testcaseasdasd";
  libmem::mmgr    mm(mmgr_name, { 128 }, { 100 });
  libmem::smgr    sm(mmgr_name);

  auto seg1 = mm.STATIC_ALLOC(128);
  REQUIRE(seg1->type == libmem::SEG_TYPE::STATIC_SEGMENT);
  REQUIRE(seg1->size == 128);
  auto mm_seg_info1 = seg1->to_seginfo();
  REQUIRE(mm_seg_info1.size() == 128);
  REQUIRE(mm_seg_info1.id() == seg1->id);

  auto sm_seg_info1 = sm.register_segment(&mm_seg_info1, ec);
  REQUIRE_FALSE(ec);

  auto buffer1 = sm.bufferize(sm_seg_info1, ec);
  REQUIRE_FALSE(ec);
  REQUIRE(buffer1.first);
  REQUIRE(buffer1.second == sm_seg_info1->size());

  sm.unregister_segment(sm_seg_info1->id(), ec);
  REQUIRE_FALSE(ec);
}