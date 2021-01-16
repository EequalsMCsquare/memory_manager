#include "bins/static_bin.hpp"
#include "segment.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

std::atomic_size_t counter = 1;

namespace libmem = shm_kernel::memory_manager;

TEST_CASE("create static_bin", "[create]")
{
  libmem::static_bin bin(0, counter, 32, 200, 0);
  counter += 1;
  REQUIRE(bin.base_pshift() == 0);
  REQUIRE(bin.chunk_count() == 200);
  REQUIRE(bin.chunk_left() == 200);
  REQUIRE(bin.chunk_size() == 32);
}

TEST_CASE("when create static bin with unaligned chunk size, thorw", "[create]")
{
  REQUIRE_THROWS(libmem::static_bin(0, counter, 31, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(0, counter, 48, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(0, counter, 127, 200, 0));
  counter += 1;
  REQUIRE_NOTHROW(libmem::static_bin(0, counter, 128, 200, 0));
  counter += 1;
  REQUIRE_NOTHROW(libmem::static_bin(0, counter, 1024, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(0, counter, 1024, 200, 1111));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(0, counter, 1024, 200, 3333));
}

TEST_CASE("allocate with successful return", "[malloc]")
{
  libmem::static_bin bin(0, counter, 32, 200, 0);
  auto               seg1 = bin.malloc(128);
  REQUIRE(seg1);
  REQUIRE(seg1->addr_pshift_ == 0);
  REQUIRE(seg1->size_ == 128);
  REQUIRE(seg1->type_ == libmem::SEG_TYPE::statbin_segment);

  auto seg2 = bin.malloc(128);
  REQUIRE(seg2);
  REQUIRE(seg2->addr_pshift_ == 128);
  REQUIRE(seg2->size_ == 128);
  REQUIRE(seg2->type_ == libmem::SEG_TYPE::statbin_segment);

  auto seg3 = bin.malloc(256);
  REQUIRE(seg3);
  REQUIRE(seg3->addr_pshift_ == 256);
  REQUIRE(seg3->size_ == 256);
  REQUIRE(seg3->type_ == libmem::SEG_TYPE::statbin_segment);

  auto seg4 = bin.malloc(512);
  REQUIRE(seg4);
  REQUIRE(seg4->addr_pshift_ == 512);
  REQUIRE(seg4->size_ == 512);
  REQUIRE(seg4->type_ == libmem::SEG_TYPE::statbin_segment);
}

TEST_CASE("allocate with error return", "[malloc]")
{
  libmem::static_bin bin(0, counter, 32, 100, 0);
  counter += 1;
  // successful malloc
  auto seg1 = bin.malloc(512);
  REQUIRE(seg1);

  // insufficient memory return -1
  auto seg2 = bin.malloc(32 * 101);
  REQUIRE_FALSE(seg2);
}

TEST_CASE("free allocated buffer", "[free]")
{
  libmem::static_bin bin(0, counter, 32, 200, 0);
  counter += 1;

  constexpr size_t ALLOC_SIZE = 256;

  // malloc
  auto seg1 = bin.malloc(ALLOC_SIZE);
  REQUIRE(seg1);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  auto seg2 = bin.malloc(ALLOC_SIZE);
  REQUIRE(seg2);
  REQUIRE(bin.chunk_left() == 200 - 2 * (ALLOC_SIZE / bin.chunk_size()));

  // free
  bin.free(seg1);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 0 - 64 bytes
  auto seg3 = bin.malloc(64);
  REQUIRE(seg3);
  REQUIRE(bin.chunk_left() == 200 - 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 64 - 128 bytes
  auto seg4 = bin.malloc(64);
  REQUIRE(seg4);
  REQUIRE(bin.chunk_left() == 200 - 2 * 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 128 - 192 bytes
  auto seg5 = bin.malloc(64);
  REQUIRE(seg5);
  REQUIRE(bin.chunk_left() == 200 - 2 * 3 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 192 - 256 bytes
  auto seg6 = bin.malloc(64);
  REQUIRE(seg6);
  REQUIRE(bin.chunk_left() == 200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 512 - 768
  auto seg7 = bin.malloc(256);
  REQUIRE(seg7);
  REQUIRE(bin.chunk_left() ==
          200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()) * 2);
}

TEST_CASE("ilegal range segment free error", "[free]")
{
  libmem::static_bin bin(0, counter, 32, 10, 0);
  counter++;
  auto __seg   = std::make_shared<libmem::base_segment>();
  __seg->type_ = libmem::SEG_TYPE::statbin_segment;

  // ilegal ptr
  __seg->addr_pshift_ = 400;
  __seg->size_        = 0;
  auto rv1            = bin.free(__seg);
  REQUIRE(rv1 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal ptr
  __seg->addr_pshift_ = 800;
  __seg->size_        = 0;
  auto rv2            = bin.free(__seg);
  REQUIRE(rv2 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  __seg->addr_pshift_ = 0;
  __seg->size_        = 800;
  auto rv3            = bin.free(__seg);
  REQUIRE(rv3 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  __seg->addr_pshift_ = 0;
  __seg->size_        = 400;
  auto rv4            = bin.free(__seg);
  REQUIRE(rv4 == -1);
  REQUIRE(bin.chunk_left() == 10);
}

TEST_CASE("double free error", "[free]")
{
  libmem::static_bin bin(0, counter, 32, 10, 0);
  counter++;

  auto __seg   = std::make_shared<libmem::base_segment>();
  __seg->type_ = libmem::SEG_TYPE::statbin_segment;
  // double free
  __seg->addr_pshift_ = 0;
  __seg->size_        = 128;
  auto rv1            = bin.free(__seg);
  REQUIRE(rv1 == -2);
  REQUIRE(bin.chunk_left() == 10);

  auto seg = bin.malloc(128);
  REQUIRE(seg);
  REQUIRE(seg->addr_pshift_ == 0);
  REQUIRE(bin.chunk_left() == 6);

  seg->size_ = 256;
  auto rv2   = bin.free(seg);
  REQUIRE(rv2 == -2);
  REQUIRE(bin.chunk_left() == 6);
}