#define CATCH_CONFIG_MAIN
#include "batch.hpp"
#include "mem_literals.hpp"
#include <catch2/catch.hpp>

namespace libmem = shm_kernel::memory_manager;

SCENARIO("Make use of an existing batch", "[batch]")
{
  GIVEN("A 56MB batch and a segment_counter")
  {
    std::atomic_size_t segment_counter{ 0 };
    libmem::batch      batch(
      "test_arena", 1, segment_counter, 4_KB, 128_KB, 8_KB, 64);
    INFO("batch total bytes: " << (batch.total_bytes() / 1024 / 1024) << " MB");
    REQUIRE(batch.min_chunksz() == 4_KB);
    REQUIRE(batch.max_chunksz() <= 256_KB);
    REQUIRE(batch.id() == 1);
    REQUIRE(batch.arena_name().compare("test_arena") == 0);

    GIVEN("A 16KB segment")
    {
      auto seg = batch.allocate(16_KB);
      REQUIRE(seg->batch_id_ == 1);
      REQUIRE(seg->id_ == 0);
      REQUIRE(batch.arena_name().compare("test_arena") == 0);
      REQUIRE(seg->size_ == 16_KB);
      REQUIRE(seg->bin_id_ == 0);

      THEN("the segment counter should increase")
      {
        REQUIRE(segment_counter == 1);
      }
      WHEN("The segment is deallocated")
      {
        auto rv = batch.deallocate(seg);
        THEN("There should be no error") { REQUIRE(rv == 0); }
      }

      GIVEN("A 20KB segment")
      {
        auto seg = batch.allocate(20_KB);
        REQUIRE(seg->batch_id_ == 1);
        REQUIRE(seg->arena_name_.compare("test_arena") == 0);
        REQUIRE(seg->id_ == 1);
        REQUIRE(seg->size_ == 20_KB);
        REQUIRE(seg->bin_id_ == 2);

        THEN("the segment counter should increase")
        {
          REQUIRE(segment_counter == 2);
        }
        WHEN("The segment is deallocated")
        {
          auto rv = batch.deallocate(seg);
          THEN("There should be no error") { REQUIRE(rv == 0); }
        }

        GIVEN("A 28_KB segment")
        {
          auto seg = batch.allocate(28_KB);
          REQUIRE(seg->batch_id_ == 1);
          REQUIRE(seg->arena_name_.compare("test_arena") == 0);
          REQUIRE(seg->id_ == 2);
          REQUIRE(seg->size_ == 28_KB);
          REQUIRE(seg->bin_id_ == 3);

          THEN("the segment counter should increase")
          {
            REQUIRE(segment_counter == 3);
          }
          WHEN("The segment is deallocated")
          {
            auto rv = batch.deallocate(seg);
            THEN("There should be no error") { REQUIRE(rv == 0); }
          }

          GIVEN("A 32_KB segment")
          {
            auto seg = batch.allocate(32_KB);
            REQUIRE(seg->batch_id_ == 1);
            REQUIRE(seg->arena_name_.compare("test_arena") == 0);
            REQUIRE(seg->id_ == 3);
            REQUIRE(seg->size_ == 32_KB);
            REQUIRE(seg->bin_id_ == 0);

            THEN("the segment counter should increase")
            {
              REQUIRE(segment_counter == 4);
            }
            WHEN("The segment is deallocated")
            {
              auto rv = batch.deallocate(seg);
              THEN("There should be no error") { REQUIRE(rv == 0); }
            }
          }
        }
      }
    }

    WHEN("Allocate size is > than max bin's chunk size * 8")
    {
      REQUIRE_THROWS(batch.allocate(9 * batch.max_chunksz()));
      REQUIRE_THROWS(batch.allocate(16 * batch.max_chunksz()));
      REQUIRE_THROWS(batch.allocate(32 * batch.max_chunksz()));
      REQUIRE_THROWS(batch.allocate(32_MB));
    }

    GIVEN("Give an ivalid batch_id segment")
    {
      auto seg       = std::make_shared<libmem::static_segment>();
      seg->batch_id_ = 100;
      WHEN("deallocate a invalid segment")
      {
        auto rv = batch.deallocate(seg);

        THEN("Error should happen") { REQUIRE(rv == -1); }
      }
    }
  }
}
