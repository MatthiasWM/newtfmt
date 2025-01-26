
#include "part_entry.h"

#include "package_bytes.h"
#include "part_data.h"

using namespace ns;

/**
 Hold all the Part attributes, but not the Part Data.
 \param[in] ix index within the Package Part list
 */
PartEntry::PartEntry(int ix)
  : index_(ix) { }

/**
 Size of the Part Data block.
 \return bytes in the Part Data
 */
int PartEntry::size()
{
  return size_;
}

/**
 Index within the Package Part list.
 \return 0-based index
 */
int PartEntry::index()
{
  return index_;
}

/**
 Read the Part attributes of the Package
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartEntry::load(PackageBytes &p) {
  offset_ = p.get_uint();
  size_ = p.get_uint();
  size2_ = p.get_uint();
  if (size_ != size2_)
    std::cout << "WARNING: Part Entry " << index_ << ": Size and size2 differ.\n";
  type_ = p.get_cstring(4, false);
  // 'form' 'book' 'dict' 'auto' 'comm'
  if (type_=="book" || type_=="dict" || type_=="comm")
    std::cout << "WARNING: Part Entry " << index_ << ": unsupported type \"" << type_ << "\"\n";
  if (type_!="form" && type_!="book" && type_!="dict" && type_!="comm")
    std::cout << "WARNING: Part Entry " << index_ << ": unknown type \"" << type_ << "\"\n";
  reserved_ = p.get_uint();
  flags_ = p.get_uint();
  if ((flags_ & 3) == 3)
    std::cout << "WARNING: Part Entry " << index_ << ": unknown type in flags: "
    << (flags_ & 3) << std::endl;
  if (flags_ & 0xfffffe0c)
    std::cout << "WARNING: Part Entry " << index_ << ": unknown flag: "
    << std::setw(8) << std::setfill('0') << std::hex
    << (flags_ & 0xfffffe0c) << std::dec << std::endl;
  info_offset_ = p.get_ushort();
  info_length_ = p.get_ushort();
  compressor_offset_ = p.get_ushort();
  compressor_length_ = p.get_ushort();

  if ((flags_ & 3) == 1) // kNOSPart
    part_data_ = std::make_shared<PartDataNOS>(*this);
  else
    part_data_ = std::make_shared<PartDataGeneric>(*this);

  return 0;
}

/**
 Read the optional Info field.
 If the Package was created with NTK, this defaults to
 "A Newton Toolkit application".
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartEntry::loadInfo(PackageBytes &p) {
  if (info_length_ > 0)
    info_ = p.get_cstring(info_length_, false);
  return 0;
}

/**
 Read the part data using an interpreter for the format as set in the flags.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartEntry::loadPartData(PackageBytes &p) {
  return part_data_->load(p);
}

/**
 Write the Package Part attributes in ARM32 assembler format.
 \param[in] f output stream
 \return number of bytes written
 */
int PartEntry::writeAsm(std::ofstream &f) {
  f << "@ ===== Part Entry " << index_ << std::endl;
  f << "\t.int\t" << offset_ << "\t@ offset" << std::endl;
  f << "\t.int\t" << size_ << "\t@ size" << std::endl;
  f << "\t.int\t" << size2_ << "\t@ size2" << std::endl;
  f << "\t.ascii\t\"" << type_ << "\"\t@ type\n";
  f << "\t.int\t" << reserved_ << "\t@ reserved" << std::endl;
  f << "\t.int\t0x" << std::setw(8) << std::setfill('0') << std::hex << flags_ << std::dec << "\t@ flags\n";
  static const std::string lut[] = { "kProtocolPart", "kNOSPart", "kRawPart", "UNKNOWN"};
  f << "\t\t@ " << lut[flags_ & 3] << std::endl;
  if (flags_ & 0x000001f0) {
    f << "\t\t@";
    if (flags_ & 0x00000010) f << " kAutoLoadPartFlag";
    if (flags_ & 0x00000020) f << " kAutoRemovePartFlag";
    if (flags_ & 0x00000040) f << " kCompressedFlag";
    if (flags_ & 0x00000080) f << " kNotifyFlag";
    if (flags_ & 0x00000100) f << " kAutoCopyFlag";
    f << std::endl;
  }
  if (flags_ & 0xfffffe0c)
    f << "\t@ WARNING unknown flag: "
    << std::setw(8) << std::setfill('0') << std::hex
    << (flags_ & 0xfffffe0c) << std::dec << std::endl;
  f << "\t.short\t" << info_offset_ << ", " << info_length_ << "\t@ info" << std::endl;
  f << "\t.short\t" << compressor_offset_ << ", " << compressor_length_ << "\t@ compressor" << std::endl;
  f << std::endl;
  return 32;
}

/**
 Write the optional Info filed in ARM32 assembler format.
 \param[in] f output stream
 \return number of bytes written
 */
int PartEntry::writeAsmInfo(std::ofstream &f) {
  f << "@ ----- Part " << index_ << " Info" << std::endl;
  f << "part" << index_ << "info_start:" << std::endl;
  if (info_length_)
    f << "\t.ascii\t\"" << info_ << "\"\t@ info" << std::endl;
  f << "part" << index_ << "info_end:" << std::endl << std::endl;
  return info_length_;
}

/**
 Write the Part Data.
 \param[in] f output stream
 \return number of bytes written
 */
int PartEntry::writeAsmPartData(std::ofstream &f) {
  return part_data_->writeAsm(f);
}

