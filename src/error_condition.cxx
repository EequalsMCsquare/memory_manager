#include "error_condition.hpp"

struct MmgrErrorCategory : public std::error_category
{
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char*
MmgrErrorCategory::name() const noexcept
{
  return "memory_manager";
}

std::string
MmgrErrorCategory::message(int ev) const
{
  switch (static_cast<MmgrErrc>(ev)) {
    case MmgrErrc::NoError:
      return "no error";
    case MmgrErrc::NoMemory:
      return "insufficient memory!";
    case MmgrErrc::SegmentNotFound:
      return "unable to locate segment!";
    case MmgrErrc::SegmentTypeUnmatched:
      return "segment's type unmatched!";
    case MmgrErrc::NullptrBuffer:
      return "buffer is nullptr!";
    case MmgrErrc::NullptrSegment:
      return "segment is nullptr!";
    case MmgrErrc::DuplicatedKey:
      return "segment key already exist!";
    case MmgrErrc::UnableToCreateShm:
      return "unable to create shared memory object!";
    case MmgrErrc::MmgrNameUnmatch:
      return "segment's mmgr_name unmatched!";
    case MmgrErrc::ShmHandleNotFound:
      return "unable to locate shm_handle!";
    case MmgrErrc::BinUnmatched:
      return "segment's bin unmatched!";
    case MmgrErrc::BatchUnmatched:
      return "segment's batch unmatched!";
    case MmgrErrc::IllegalSegmentRange:
      return "segment is in invalid range!";
    case MmgrErrc::SegmentDoubleFree:
      return "segment is already marked as freed!";
    case MmgrErrc::TooBigForStaticBin:
      return "size is too big for static bin!";
    case MmgrErrc::NoSuitableStaticBin:
      return "no available static bin!";
    case MmgrErrc::UnableToRegisterSegment:
      return "unable to insert segment into segment table!";
    default:
      return "unknown error";
  }
}

const MmgrErrorCategory TheMmgrErrorCategory{};

std::error_code
make_error_code(MmgrErrc ec)
{
  return {static_cast<int>(ec), TheMmgrErrorCategory };
}
