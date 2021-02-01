#pragma once

#include <system_error>

enum class MmgrErrc
{
  NoError = 0,
  NoMemory,
  SegmentNotFound,
  SegmentTypeUnmatched,
  NullptrBuffer,
  NullptrSegment,
  DuplicatedKey,
  UnableToCreateShm,
  MmgrNameUnmatch,
  ShmHandleNotFound,
  BinUnmatched,
  BatchUnmatched,
  IllegalSegmentRange,
  SegmentDoubleFree,
  TooBigForStaticBin,
  NoSuitableStaticBin,
  UnableToRegisterSegment,

};

namespace std {
template<>
struct is_error_code_enum<MmgrErrc> : true_type
{};
}

std::error_code make_error_code(MmgrErrc);
