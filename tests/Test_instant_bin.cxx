#include "bins/instant_bin.hpp"
#define CATCH_CONFIG_MAIN
#include "mem_literals.hpp"
#include <catch2/catch.hpp>

TEST_CASE("create instant bin", "[create]")
{
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");
  REQUIRE(bin.shmhdl_count() == 0);
}

TEST_CASE("instant bin malloc", "[malloc]")
{
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");
  REQUIRE(bin.shmhdl_count() == 0);

  auto seg1_id = bin.malloc(4_MB);
  REQUIRE(seg1_id == 0);
  REQUIRE(bin.shmhdl_count() == 1);
  auto seg1_shmhdl = bin.get_shmhdl_ptr(seg1_id);
  REQUIRE(seg1_shmhdl.use_count() == 2);
  REQUIRE(seg1_shmhdl->ref_count() == 1);
  REQUIRE(seg1_shmhdl->nbytes() >= 4_MB);

  auto seg2_id = bin.malloc(16_MB);
  REQUIRE(seg2_id == 1);
  REQUIRE(bin.shmhdl_count() == 2);
  auto seg2_shmhdl = bin.get_shmhdl_ptr(seg2_id);
  REQUIRE(seg2_shmhdl.use_count() == 2);
  REQUIRE(seg2_shmhdl->ref_count() == 1);
  REQUIRE(seg2_shmhdl->nbytes() >= 16_MB);

  auto seg3_id = bin.malloc(128_MB);
  REQUIRE(seg3_id == 2);
  REQUIRE(bin.shmhdl_count() == 3);
  auto seg3_shmhdl = bin.get_shmhdl_ptr(seg3_id);
  REQUIRE(seg3_shmhdl.use_count() == 2);
  REQUIRE(seg3_shmhdl->ref_count() == 1);
  REQUIRE(seg3_shmhdl->nbytes() >= 128_MB);
}

TEST_CASE("instant bin free", "[free]")
{
  std::atomic_size_t  segment_counter = 0;
  libmem::instant_bin bin(segment_counter, "test_arena");

  auto seg1_id = bin.malloc(4_MB);
  REQUIRE(seg1_id == 0);
  {
    auto seg1_shmhdl = bin.get_shmhdl_ptr(seg1_id);
    REQUIRE(seg1_shmhdl->nbytes() >= 4_MB);
    REQUIRE(seg1_shmhdl->ref_count() == 1);
    REQUIRE(bin.size() == 1);
  }

  bin.free(seg1_id);
  REQUIRE(bin.shmhdl_count() == 0);

  auto seg2_id = bin.malloc(128_MB);
  REQUIRE(seg2_id == 1);
  {
    auto seg2_shmhdl = bin.get_shmhdl_ptr(seg2_id);
    REQUIRE(seg2_shmhdl->nbytes() >= 128_MB);
    REQUIRE(seg2_shmhdl->ref_count() == 1);
    REQUIRE(bin.size() == 1);
  }

  bin.free(seg2_id);
  REQUIRE(bin.shmhdl_count() == 0);
}
