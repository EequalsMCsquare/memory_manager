#include "bins/static_bin.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

std::atomic_size_t counter = 1;

TEST_CASE("create static_bin", "[create]")
{
  libmem::static_bin bin(counter, 32, 200, 0);
  counter += 1;
  REQUIRE(bin.base_pshift() == 0);
  REQUIRE(bin.chunk_count() == 200);
  REQUIRE(bin.chunk_left() == 200);
  REQUIRE(bin.chunk_size() == 32);
}

TEST_CASE("when create static bin with unaligned chunk size, thorw", "[create]")
{
  REQUIRE_THROWS(libmem::static_bin(counter, 31, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(counter, 48, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(counter, 127, 200, 0));
  counter += 1;
  REQUIRE_NOTHROW(libmem::static_bin(counter, 128, 200, 0));
  counter += 1;
  REQUIRE_NOTHROW(libmem::static_bin(counter, 1024, 200, 0));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(counter, 1024, 200, 1111));
  counter += 1;
  REQUIRE_THROWS(libmem::static_bin(counter, 1024, 200, 3333));
}

TEST_CASE("allocate with successful return", "[malloc]")
{
  libmem::static_bin bin(counter, 32, 200, 0);
  counter += 1;
  auto ptr1 = bin.malloc(128);
  REQUIRE(ptr1 > -1);
  REQUIRE(ptr1 == 0);

  auto ptr2 = bin.malloc(128);
  REQUIRE(ptr2 > -1);
  REQUIRE(ptr2 == 128);

  auto ptr3 = bin.malloc(256);
  REQUIRE(ptr3 > -1);
  REQUIRE(ptr3 == 256);
}

TEST_CASE("allocate with error return", "[malloc]")
{
  libmem::static_bin bin(counter, 32, 100, 0);
  counter += 1;
  // successful malloc
  auto ptr1 = bin.malloc(512);
  REQUIRE(ptr1 >= 0);

  // insufficient memory return -1
  auto ptr2 = bin.malloc(32 * 101);
  REQUIRE(ptr2 == -1);
}

TEST_CASE("free allocated buffer", "[free]")
{
  libmem::static_bin bin(counter, 32, 200, 0);
  counter += 1;

  constexpr size_t ALLOC_SIZE = 256;

  // malloc
  auto ptr1 = bin.malloc(ALLOC_SIZE);
  REQUIRE(ptr1 >= 0);
  REQUIRE(ptr1 == 0);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  auto ptr2 = bin.malloc(ALLOC_SIZE);
  REQUIRE(ptr2 >= 0);
  REQUIRE(ptr2 == 256);
  REQUIRE(bin.chunk_left() == 200 - 2 * (ALLOC_SIZE / bin.chunk_size()));

  // free
  bin.free(ptr1, 256);
  REQUIRE(bin.chunk_left() == 200 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 0 - 64 bytes
  auto ptr3 = bin.malloc(64);
  REQUIRE(ptr3 >= 0);
  REQUIRE(ptr3 == 0);
  REQUIRE(bin.chunk_left() == 200 - 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 64 - 128 bytes
  auto ptr4 = bin.malloc(64);
  REQUIRE(ptr4 >= 0);
  REQUIRE(ptr4 == 64);
  REQUIRE(bin.chunk_left() == 200 - 2 * 2 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 128 - 192 bytes
  auto ptr5 = bin.malloc(64);
  REQUIRE(ptr5 >= 0);
  REQUIRE(ptr5 == 128);
  REQUIRE(bin.chunk_left() == 200 - 2 * 3 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 192 - 256 bytes
  auto ptr6 = bin.malloc(64);
  REQUIRE(ptr6 >= 0);
  REQUIRE(ptr6 == 192);
  REQUIRE(bin.chunk_left() == 200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()));

  // malloc with 512 - 768
  auto ptr7 = bin.malloc(256);
  REQUIRE(ptr7 >= 0);
  REQUIRE(ptr7 == 512);
  REQUIRE(bin.chunk_left() ==
          200 - 2 * 4 - (ALLOC_SIZE / bin.chunk_size()) * 2);
}

TEST_CASE("ilegal range segment free error", "[free]")
{
  libmem::static_bin bin(counter, 32, 10, 0);
  counter++;

  // ilegal ptr
  auto rv1 = bin.free(400, 0);
  REQUIRE(rv1 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal ptr
  auto rv2 = bin.free(800, 0);
  REQUIRE(rv2 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  auto rv3 = bin.free(0, 800);
  REQUIRE(rv3 == -1);
  REQUIRE(bin.chunk_left() == 10);

  // ilegal segment
  auto rv4 = bin.free(0, 400);
  REQUIRE(rv4 == -1);
  REQUIRE(bin.chunk_left() == 10);
}

// TEST_CASE("double free error", "[free]")
// {
//   libmem::static_bin bin(counter, 32, 10, 0);
//   counter++;

//   // double free
//   auto rv1 = bin.free(0, 128);
//   REQUIRE(rv1 == -2);
//   REQUIRE(bin.chunk_left() == 10);

//   auto ptr = bin.malloc(128);
//   REQUIRE(ptr >= 0);
//   REQUIRE(ptr == 0);

//   auto rv2 = bin.free(ptr, 256);
//   REQUIRE(rv2 == -2);
//   REQUIRE(bin.chunk_left() == 10);
// }