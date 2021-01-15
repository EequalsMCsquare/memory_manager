#include "segment.hpp"
#define CATCH_CONFIG_MAIN
#include <arena.hpp>
#include <catch2/catch.hpp>
#include <mem_literals.hpp>

TEST_CASE("create arena", "[create]")
{
  libmem::arena arena("test_arena");
  REQUIRE(arena.batch_count() == 1);
  REQUIRE(arena.name().compare("test_arena") == 0);
}

TEST_CASE("ceate segment using static bin", "[malloc]")
{
  libmem::arena arena("test_arena");
  auto          seg1 = arena.allocate(2_KB);
  REQUIRE(seg1->arena_name_.compare("test_arena") == 0);
  REQUIRE(seg1->id_ == 0);
  REQUIRE(seg1->type_ == libmem::SEG_TYPE::statbin_segment);
  REQUIRE(seg1->size_ >= 2_KB);

  auto seg2 = arena.allocate(4_KB);
  REQUIRE(seg2->arena_name_.compare("test_arena") == 0);
  REQUIRE(seg2->id_ == 1);
  REQUIRE(seg2->type_ == libmem::SEG_TYPE::statbin_segment);
  REQUIRE(seg2->size_ >= 4_KB);
}

TEST_CASE("ceate segment using instant bin", "[malloc]")
{
  libmem::arena arena("test_arena");
  auto          seg1 = arena.allocate(32_MB);
  REQUIRE(seg1);
  REQUIRE(seg1->arena_name_ == "test_arena");
  REQUIRE(seg1->type_ == libmem::SEG_TYPE::instbin_segment);
  REQUIRE(seg1->size_ >= 32_MB);

  auto seg2 = arena.allocate(2_GB);
  REQUIRE(seg2);
  REQUIRE(seg2->arena_name_ == "test_arena");
  REQUIRE(seg2->type_ == libmem::SEG_TYPE::instbin_segment);
  REQUIRE(seg2->size_ >= 2_GB);
}