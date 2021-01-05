#define CATCH_CONFIG_MAIN
#include "batch.hpp"
#include "mem_literals.hpp"
#include <catch2/catch.hpp>

TEST_CASE("create a batch", "[create]")
{
  std::atomic_size_t segment_counter{ 0 };
  libmem::batch      batch("test_arena", 1, 1_KB, 256_KB, 8_KB, 50);
  std::cout << "batch total bytes: " << batch.total_bytes() << std::endl;
  REQUIRE(batch.min_chunksz() == 1_KB);
  REQUIRE(batch.max_chunksz() <= 256_KB);
}