/*
 * newtfmt, working title, a Newton Script file reader and writer
 * Copyright (C) 2025  Matthias Melcher
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "package.h"

#include "package_bytes.h"
#include "part_entry.h"
#include "tools/tools.h"

#include "nos/objects.h"

#include <cassert>

using namespace pkg;

/** \class pkg::Package
 Read, store, and write the binary data in NewtonScript Package format.
 */

/**
 Load the entire package and store the content in memory.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int Package::load() {
  if (!pkg_bytes_) {
    std::cout << "ERROR: package bytes not initialized.\n";
    return -1;
  }
  if (!pkg_bytes_->size()) {
    std::cout << "ERROR: no package bytes loaded, size is 0.\n";
    return -1;
  }
  pkg_bytes_->rewind();
  signature_ = pkg_bytes_->get_cstring(8, false);
  if ((signature_ != "package0") && (signature_ != "package1")) {
    std::cout << "ERROR: unknown signature \"" << signature_ << "\"\n";
    return -1;
  }
  type_ = pkg_bytes_->get_cstring(4, false);
  flags_ = pkg_bytes_->get_uint();
  if (flags_ & 0x08ffffff)
    std::cout << "WARNING: unknown flag: "
    << std::setw(4) << std::setfill('0') << std::hex
    << (flags_ & 0x08ffffff) << std::endl;
  if (flags_ & 0x01000000)
    std::cout << "INFO: Package certified to run on Schlumberger Watson." << std::endl;
  version_ = pkg_bytes_->get_uint();
  copyright_start_ = pkg_bytes_->get_ushort();
  if (copyright_start_ != 0)
    std::cout << "WARNING: Copyright offset should be 0.\n";
  copyright_length_ = pkg_bytes_->get_ushort();
  name_start_ = pkg_bytes_->get_ushort();
  // TODO: the following is a bad assumption and creates a wrong offset by using the wrong labels
  if (name_start_ != copyright_start_ + copyright_length_)
    std::cout << "WARNING: Name offset should be " << copyright_start_ + copyright_length_
    << ", but it's " << name_start_ << ".\n";
  name_length_ = pkg_bytes_->get_ushort();
  if (name_length_ == 0)
    std::cout << "WARNING: Name length can't be 0.\n";
  size_ = pkg_bytes_->get_uint();
  if (size_ < pkg_bytes_->size())
    std::cout << "WARNING: size entry does not match file size (" << size_ << "!=" << pkg_bytes_->size() << ").\n";
  if (size_ > pkg_bytes_->size()) {
    std::cout << "ERROR: expected size is less than file size, file is cropped (" << size_ << "!=" << pkg_bytes_->size() << ").\n";
    return -1;
  }
  date_ = pkg_bytes_->get_uint();
  reserved2_ = pkg_bytes_->get_uint();
//  TODO: this field is documented as 0, but it is set in many packages, and seem to be holding the "Modification Data"
//  if (reserved2_ != 0)
//    std::cout << "WARNING: Reserved2 should be 0, but it is " << reserved2_ << " = 0x"
//    << std::setw(8) << std::setfill('0') << std::hex << reserved2_ << std::dec << ".\n";
  reserved3_ = pkg_bytes_->get_uint();
  if (reserved3_ != 0)
    std::cout << "WARNING: Reserved3 should be 0.\n";
  directory_size_ = pkg_bytes_->get_uint();
  num_parts_ = pkg_bytes_->get_uint();
  if (num_parts_ > 32)
    std::cout << "WARNING: Unlikely number of parts (" << num_parts_ << ").\n";
  for (int i = 0; i < (int)num_parts_; ++i) {
    part_.push_back(std::make_shared<PartEntry>(i));
    part_[i]->load(*pkg_bytes_);
  }
  vdata_start_ = pkg_bytes_->tell();
  if (copyright_length_) {
    copyright_ = pkg_bytes_->get_ustring(copyright_length_/2-1);
  }
  if (name_length_) {
    name_ = pkg_bytes_->get_ustring(name_length_/2-1);
  }
  for (auto &part: part_) part->loadInfo(*pkg_bytes_);
  // NTK sneaks a message into the variable data area after the last info
  // and before the relocation data and parts start.
  // "Newton™ ToolKit Package © 1992-1997, Apple Computer, Inc."
  info_length_ = directory_size_ - pkg_bytes_->tell(); // 58 bytes + 2 bytes padding
  info_ = pkg_bytes_->get_data(info_length_);
//  std::string info((char*)&info_[0], info_length_);
//  std::cout << "PackageInfo: " << info << std::endl;

  // Relocation Data if kRelocationFlag is set
  if (flags_ & 0x04000000) {
    relocation_data_.load(*pkg_bytes_);
//    std::cout << "WARNING: Relocation Data not supported." << std::endl;
  }

  // Part Data
  for (auto &part: part_) part->loadPartData(*pkg_bytes_);

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
#if 0
  f << "\t.int\t" << size_ << "\t@ size\n";
#else
  f << "\t.int\tpackage_end-package_start\t@ size\n";
#endif
  f << "\t.int\t0x" << std::setw(8) << std::hex << date_ << std::dec << "\t@ date\n";
  f << "\t.int\t0x" << std::setw(8) << std::hex << reserved2_ << std::dec << "\t@ reserverd2\n";
  f << "\t.int\t0x" << std::setw(8) << std::hex << reserved3_ << std::dec << "\t@ reserverd3\n";
#if 0
  f << "\t.int\t" << directory_size_ << "\t@ directory_size\n";
#else
  f << "\t.int\tdirectory_size\t@ directory_size\n";
#endif
  f << "\t.int\t" << num_parts_ << "\t@ num_parts\n";
  f << std::endl;
  int bytes = 52;
  for (int i = 0; i < (int)num_parts_; ++i) {
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

  f << "directory_size:" << std::endl << std::endl;


  // Relocation Data if kRelocationFlag is set
  if (flags_ & 0x04000000) {
    bytes += relocation_data_.writeAsm(f);
  }

  f << "@ ===== Package Parts" << std::endl << std::endl;

  for (auto &part: part_) bytes += part->writeAsmPartData(f);

  f << "@ ===== Package End" << std::endl;

  return bytes;
}

/**
 Compare the package with the other package.
 \param[in] other the other package
 \return 0 if they are the same.
 */
int Package::compare(Package &other) {
  int ret = 0;
  if (signature_ != other.signature_) {
    std::cout << "WARNING: Package signatures differ!" << std::endl;
    ret = -1;
  }
  if (type_ != other.type_) {
    std::cout << "WARNING: Package type texts differ!" << std::endl;
    ret = -1;
  }
  if (flags_ != other.flags_) {
    std::cout << "WARNING: Package flags differ!" << std::endl;
    ret = -1;
  }
  if (version_ != other.version_) {
    std::cout << "WARNING: Package versions differ!" << std::endl;
    ret = -1;
  }
  if (copyright_ != other.copyright_) {
    std::cout << "WARNING: Package copyright messages differ!" << std::endl;
    ret = -1;
  }
  if (name_ != other.name_) {
    std::cout << "WARNING: Package names differ!" << std::endl;
    ret = -1;
  }
  if (size_ != other.size_) {
    std::cout << "WARNING: Package sizes differ!" << std::endl;
    ret = -1;
  }
  if (date_ != other.date_) {
    std::cout << "WARNING: Package creation dates differ!" << std::endl;
    ret = -1;
  }
  if (num_parts_ != other.num_parts_) {
    std::cout << "WARNING: Number of parts in package differ!" << std::endl;
    return -1;
  }
  for (size_t i=0; i<part_.size(); ++i) {
    ret = part_[i]->compare(*other.part_[i]);
    if (ret != 0) break;
  }
  return ret;
}


/**
 Load a Package file and read the internal data representation.
 \param[in] package_file_name path and name
 \return 0 if successful
 */
int Package::load(const std::string &package_file_name)
{
  file_name_ = package_file_name;
  std::ifstream source_file { package_file_name, std::ios::binary };
  if (source_file) {
    pkg_bytes_ = std::make_shared<PackageBytes>();
    pkg_bytes_->assign(std::istreambuf_iterator<char>{source_file}, {});
    file_name_ = package_file_name;
//    std::cout << "readPackage: \"" << file_name_ << "\" package read (" << pkg_bytes_->size() << " bytes)." << std::endl;
    return load();
  }
  std::cout << "readPackage: Unable to read file \"" << package_file_name << "\"." << std::endl;
  return -1;
}


/**
 Write a Package as an ARM32 assembler file.
 \param[in] assembler_file_name path and name
 \return 0 if successful
 */
int Package::writeAsm(const std::string &assembler_file_name)
{
  std::ofstream asm_file { assembler_file_name };
  if (asm_file.fail()) {
    std::cout << "writeAsm: Unable to write assembler file \"" << assembler_file_name << "\"." << std::endl;
    return -1;
  }

  asm_file << "@" << std::endl;
  asm_file << "@ Assembler file generated from Newton Package" << std::endl;
  asm_file << "@" << std::endl << std::endl;

  asm_file << "\t.macro\tref_magic index\n"
  << "\t.int\t((\\index)<<2)|3\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_integer value\n"
  << "\t.int\t((\\value)<<2)\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_pointer label\n"
  << "\t.int\t\\label + 1\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_pointer_invalid offset\n"
  << "\t.int\t\\offset\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_unichar value\n"
  << "\t.int\t((\\value)<<4)|10\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_nil\n"
  << "\t.int\t0x00000002\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tref_true\n"
  << "\t.int\t0x0000001a\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tnscmd1 cmd, data\n"
  << "\t.byte\t(\\cmd<<3)|\\data\n"
  << "\t.endm\n\n";

  asm_file << "\t.macro\tnscmd3 cmd, data\n"
  << "\t.byte\t(\\cmd<<3)|0x07, \\data>>8, \\data&0x00ff\n"
  << "\t.endm\n\n";

//  asm_file << "\t.macro\tns_align n\n"
//  << "\t.space\t(( . - part_0) + 7) & ~(n-1), 0xbf\n"
//  << "\t.space\t . & 0x0f, 0xbf\n"
//  << "\t.endm\n\n";

  asm_file << "\t.file\t\"" << file_name_ << "\"" << std::endl;
  asm_file << "\t.data" << std::endl;
  asm_file << "package_start:" << std::endl << std::endl;

  int skip = writeAsm(asm_file);
  asm_file << "package_end:" << std::endl << std::endl;

  if (skip < (int)pkg_bytes_->size()) {
    std::cout << "WARNING: Package has " << pkg_bytes_->size()-skip << " more bytes than defined." << std::endl;
    asm_file << "@ ===== Extra data in file" << std::endl;
    for (auto it = pkg_bytes_->begin()+skip; it != pkg_bytes_->end(); ++it) {
      uint8_t b = *it;
      asm_file << "\t.byte\t0x"
      << std::setw(2) << std::setfill('0') << std::hex << (int)b
      << "\t@ " << (char)( ((b > 32) && (b < 127)) ? b : '.' )
      << std::endl;
    }
  }

  return 0;
}

/**
 Compare this package byte-by-byte to another package file.
 \param[in] other_package_file file path and name of the contender
 \return 0 if file content creates the same binary representation
 */
int Package::compareFile(const std::string &other_package_file) {
  std::vector<uint8_t> new_pkg;
  std::ifstream new_file { other_package_file, std::ios::binary };
  if (new_file) {
    new_pkg.assign(std::istreambuf_iterator<char>{new_file}, {});
    if (new_pkg == *pkg_bytes_) {
      //      std::cout << "compareBinaries: Packages are identical." << std::endl;
      //      std::cout << "OK." << std::endl;
    } else {
      int i, n = std::min((int)new_pkg.size(), (int)pkg_bytes_->size());
      for (i=0; i<n; ++i) {
        if (new_pkg[i] != pkg_bytes_->at(i)) break;
      }
      std::cout << "ERROR: compareFile: Packages differ starting at 0x"
      << std::setw(8) << std::setfill('0') << std::hex << i << std::dec
      << " = " << i << "!" << std::endl;
    }
    std::cout << std::endl;
    return 0;
  }
  std::cout << "compareFile: Unable to read new file \"" << other_package_file << "\"." << std::endl;
  return -1;
}

/**
 Compare this package to the the contents of another package file.
 \param[in] other_package_file file path and name of the contender
 \return 0 if file content creates the same binary representation
 */
int Package::compareContents(const std::string &other_package_file) {
  Package other;
  if (other.load(other_package_file)==-1) {
    std::cout << "ERROR: compareContents: Can'r read package \"" << other_package_file << "\"." << std::endl;
    return -1;
  }
  return compare(other);
}

/**
 Convert this package into a Newton OS object tree.
 \return the object tree or an error code as an integer
 */
nos::Ref Package::toNOS() {
  nos::Ref pkg = nos::AllocateFrame();
  nos::SetFrameSlot(pkg, nos::Sym("signature"), nos::MakeString(signature_));
  nos::SetFrameSlot(pkg, nos::Sym("type"), nos::MakeString(type_));
  nos::SetFrameSlot(pkg, nos::Sym("flags"), (int)flags_);
  nos::SetFrameSlot(pkg, nos::Sym("version"), (int)version_);
  nos::SetFrameSlot(pkg, nos::Sym("copyright"), nos::MakeString(copyright_));
  nos::SetFrameSlot(pkg, nos::Sym("name"), nos::MakeString(name_));
  nos::SetFrameSlot(pkg, nos::Sym("filename"), nos::MakeString(file_name_));
  nos::SetFrameSlot(pkg, nos::Sym("date"), (int)date_);
//nos::SetFrameSlot(pkg, nos::Sym("info"), nos::MakeString(std::string(info_)));
  nos::Ref parts = nos::AllocateArray(0);
  for (auto &part: part_) {
    nos::AddArraySlot(parts, part->toNOS());
  }
  nos::SetFrameSlot(pkg, nos::Sym("parts"), parts);
  return pkg;
}
