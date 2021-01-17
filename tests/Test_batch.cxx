#define CATCH_CONFIG_MAIN
#include "batch.hpp"
#include "mem_literals.hpp"
#include <catch2/catch.hpp>

namespace libmem = shm_kernel::memory_manager;
TEST_CASE("create a batch", "[create]")
{
  std::atomic_size_t segment_counter{ 0 };
  libmem::batch      batch("test_arena", 1, 4_KB, 128_KB, 8_KB, 64);
  std::cout << "batch total bytes: " << batch.total_bytes() / 1024 / 1024
            << " MB" << std::endl;
  REQUIRE(batch.min_chunksz() == 4_KB);
  REQUIRE(batch.max_chunksz() <= 256_KB);
  REQUIRE(batch.id() == 1);
}

TEST_CASE("allocate a segment", "[allocate]")
{
  std::atomic_size_t segment_counter{ 0 };
  libmem::batch      batch("test_arena2", 1, 4_KB, 128_KB, 8_KB, 64);
  REQUIRE(batch.min_chunksz() == 4_KB);
  REQUIRE(batch.max_chunksz() <= 256_KB);
  REQUIRE(batch.id() == 1);

  auto seg1 = batch.allocate(16_KB);
  REQUIRE(seg1->batch_id_ == 1);
  REQUIRE(std::strcmp("test_arena2", seg1->arena_name_.data()) == 0);
  REQUIRE(seg1->id_ == 0);
  REQUIRE(seg1->size_ == 16_KB);
  REQUIRE(seg1->bin_id_ == 0);

  auto seg2 = batch.allocate(20_KB);
  REQUIRE(seg2->batch_id_ == 1);
  REQUIRE(std::strcmp("test_arena2", seg2->arena_name_.data()) == 0);
  REQUIRE(seg2->id_ == 1);
  REQUIRE(seg2->size_ == 20_KB);
  REQUIRE(seg2->bin_id_ == 2);

  auto seg3 = batch.allocate(28_KB);
  REQUIRE(seg3->batch_id_ == 1);
  REQUIRE(std::strcmp("test_arena2", seg3->arena_name_.data()) == 0);
  REQUIRE(seg3->id_ == 2);
  REQUIRE(seg3->size_ == 28_KB);
  REQUIRE(seg3->bin_id_ == 3);

  auto seg4 = batch.allocate(32_KB);
  REQUIRE(seg4->batch_id_ == 1);
  REQUIRE(std::strcmp("test_arena2", seg4->arena_name_.data()) == 0);
  REQUIRE(seg4->id_ == 3);
  REQUIRE(seg4->size_ == 32_KB);

  REQUIRE_THROWS(batch.allocate(128_KB * 8));
  REQUIRE_THROWS(batch.allocate(2_MB));
}

TEST_CASE("deallocate a segment", "[deallocate]")
{
  std::atomic_size_t segment_count{ 0 };
  libmem::batch      batch("test_arena3", 1, 4_KB, 128_KB, 8_KB, 64);

  auto seg1 = batch.allocate(128_KB);
  REQUIRE(batch.deallocate(seg1) == 0);

  auto seg2 = batch.allocate(32_KB);
  REQUIRE(batch.deallocate(seg2) == 0);

  auto seg3 = batch.allocate(10_KB);
  REQUIRE(batch.deallocate(seg3) == 0);
}