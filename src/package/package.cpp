
#include "package.h"

#include "package_bytes.h"
#include "part_entry.h"
#include "tools.h"

using namespace pkg;

/** \class pkg::Package
 Read, store, and write the binary data in NewtonScript Package format.
 */

/**
 Load the entire package and store the content in memory.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int Package::load(PackageBytes &p) {
  p.rewind();
  signature_ = p.get_cstring(8, false);
  if ((signature_ != "package0") && (signature_ != "package1")) {
    std::cout << "ERROR: unknown signature \"" << signature_ << "\"\n";
    return -1;
  }
  type_ = p.get_cstring(4, false);
  flags_ = p.get_uint();
  if (flags_ & 0x08ffffff)
    std::cout << "WARNING: unknown flag: "
    << std::setw(4) << std::setfill('0') << std::hex
    << (flags_ & 0x08ffffff) << std::endl;
  if (flags_ & 0x01000000)
    std::cout << "INFO: Package certified to run on Schlumberger Watson." << std::endl;
  version_ = p.get_uint();
  copyright_start_ = p.get_ushort();
  if (copyright_start_ != 0)
    std::cout << "WARNING: Copyright offset should be 0.\n";
  copyright_length_ = p.get_ushort();
  name_start_ = p.get_ushort();
  // TODO: the following is a bad assumption and creates a wrong offset by using the wrong labels
  if (name_start_ != copyright_start_ + copyright_length_)
    std::cout << "WARNING: Name offset should be " << copyright_start_ + copyright_length_
    << ", but it's " << name_start_ << ".\n";
  name_length_ = p.get_ushort();
  if (name_length_ == 0)
    std::cout << "WARNING: Name length can't be 0.\n";
  size_ = p.get_uint();
  if (size_ < p.size())
    std::cout << "WARNING: size entry does not match file size (" << size_ << "!=" << p.size() << ").\n";
  if (size_ > p.size()) {
    std::cout << "ERROR: expected size is less than file size, file is cropped (" << size_ << "!=" << p.size() << ").\n";
    return -1;
  }
  date_ = p.get_uint();
  reserved2_ = p.get_uint();
//  TODO: this field is documented as 0, but it is set in many packages, and seem to be holding the "Modification Data"
//  if (reserved2_ != 0)
//    std::cout << "WARNING: Reserved2 should be 0, but it is " << reserved2_ << " = 0x"
//    << std::setw(8) << std::setfill('0') << std::hex << reserved2_ << std::dec << ".\n";
  reserved3_ = p.get_uint();
  if (reserved3_ != 0)
    std::cout << "WARNING: Reserved3 should be 0.\n";
  directory_size_ = p.get_uint();
  num_parts_ = p.get_uint();
  if (num_parts_ > 32)
    std::cout << "WARNING: Unlikely number of parts (" << num_parts_ << ").\n";
  for (int i = 0; i < num_parts_; ++i) {
    part_.push_back(std::make_shared<PartEntry>(i));
    part_[i]->load(p);
  }
  vdata_start_ = p.tell();
  if (copyright_length_) {
    copyright_ = p.get_ustring(copyright_length_/2-1);
  }
  if (name_length_) {
    name_ = p.get_ustring(name_length_/2-1);
  }
  for (auto &part: part_) part->loadInfo(p);
  // NTK sneaks a message into the variable data area after the last info
  // and before the relocation data and parts start.
  // "Newton™ ToolKit Package © 1992-1997, Apple Computer, Inc."
  info_length_ = directory_size_ - p.tell(); // 58 bytes + 2 bytes padding
  info_ = p.get_data(info_length_);
//  std::string info((char*)&info_[0], info_length_);
//  std::cout << "PackageInfo: " << info << std::endl;

  // Relocation Data if kRelocationFlag is set
  if (flags_ & 0x04000000) {
    relocation_data_.load(p);
//    std::cout << "WARNING: Relocation Data not supported." << std::endl;
  }

  // Part Data
  for (auto &part: part_) part->loadPartData(p);

  return 0;
}

/**
 Write the Package in ARM32 assembler format.
 \todo The output is not yet symbolic. Absolute values are used
 \param[in] f output stream
 \return number of bytes written
 */
int Package::writeAsm(std::ofstream &f) {
  f << "@ ===== Package Header" << std::endl;
  f << "\t.ascii\t\"" << signature_ << "\"\t@ signature\n";
  f << "\t.ascii\t\"" << type_ << "\"\t@ type\n";
  f << "\t.int\t0x" << std::setw(8) << std::setfill('0') << std::hex << flags_ << std::dec << "\t@ flags\n";
  if (flags_ & 0xf0000000) {
    f << "\t\t@";
    if (flags_ & 0x80000000) f << " kAutoRemoveFlag";
    if (flags_ & 0x40000000) f << " kCopyProtectFlag";
    if (flags_ & 0x20000000) f << " kInvisibleFlag";
    if (flags_ & 0x10000000) f << " kNoCompressionFlag";
    f << std::endl;
  }
  if (flags_ & 0x07000000) {
    f << "\t\t@";
    if (flags_ & 0x04000000) f << " kRelocationFlag";
    if (flags_ & 0x02000000) f << " kUseFasterCompressionFlag";
    if (flags_ & 0x01000000) f << " kWatsonSignaturePresentFlag";
    f << std::endl;
  }
  if (flags_ & 0x09ffffff)
    f << "\t@ WARNING unknown flag: "
    << std::setw(4) << std::setfill('0') << std::hex
    << (flags_ & 0x09ffffff) << std::dec << std::endl;
  f << "\t.int\t" << version_ << "\t@ version\n";
  //  f << "\t.short\t" << copyright_start_ << ", " << copyright_length_ << "\t@ copyright\n";
  f << "\t.short\tpkg_copyright_start-pkg_data, pkg_copyright_end-pkg_copyright_start\t@ copyright\n";
  //  f << "\t.short\t" << name_start_ << ", " << name_length_ << "\t@ name\n";
  f << "\t.short\tpkg_name_start-pkg_data, pkg_name_end-pkg_name_start\t@ name\n";
  f << "\t.int\t" << size_ << "\t@ size\n";
  f << "\t.int\t0x" << std::setw(8) << std::hex << date_ << std::dec << "\t@ date\n";
  f << "\t.int\t0x" << std::setw(8) << std::hex << reserved2_ << std::dec << "\t@ reserverd2\n";
  f << "\t.int\t0x" << std::setw(8) << std::hex << reserved3_ << std::dec << "\t@ reserverd3\n";
  f << "\t.int\t" << directory_size_ << "\t@ directory_size\n";
  f << "\t.int\t" << num_parts_ << "\t@ num_parts\n";
  f << std::endl;
  int bytes = 52;
  for (int i = 0; i < num_parts_; ++i) {
    bytes += part_[i]->writeAsm(f);
  }
  f << "@ ===== Copyright" << std::endl;
  f << "pkg_data:" << std::endl << std::endl;

  f << "@ ----- Copyright" << std::endl;
  f << "pkg_copyright_start:" << std::endl;
  if (copyright_length_)
    bytes += write_utf16(f, copyright_);
  f << "pkg_copyright_end:" << std::endl << std::endl;

  f << "@ ----- Name" << std::endl;
  f << "pkg_name_start:" << std::endl;
  if (name_length_)
    bytes += write_utf16(f, name_);
  f << "pkg_name_end:" << std::endl << std::endl;

  for (auto &part: part_) bytes += part->writeAsmInfo(f);

  if (info_.size() > 0) {
    f << "@ ----- Package Info" << std::endl;
    bytes += write_data(f, info_);
    f << std::endl;
  }

  f << "\t.balign\t4, 0xff" << std::endl << std::endl;

  // Relocation Data if kRelocationFlag is set
  if (flags_ & 0x04000000) {
    bytes += relocation_data_.writeAsm(f);
  }

  f << "@ ===== Package Parts" << std::endl << std::endl;

  for (auto &part: part_) bytes += part->writeAsmPartData(f);

  f << "@ ===== Package End" << std::endl;

  return bytes;
}

