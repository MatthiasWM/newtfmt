


#include "part_data.h"

#include "package_bytes.h"
#include "part_entry.h"
#include "tools.h"

#include <iostream>
#include <fstream>
#include <ios>

using namespace ns;

/**
 Read the Part of the Package as raw data.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartDataGeneric::load(PackageBytes &p) {
  data_ = p.get_data(part_entry_.size());
  return 0;
}

/**
 Write raw Package Part data in ARM32 assembler format.
 \param[in] f output stream
 \return number of bytes written
 */
int PartDataGeneric::writeAsm(std::ofstream &f) {
  f << "@ ===== Part " << part_entry_.index() << " Data Generic" << std::endl;
  write_data(f, data_);
  f << "\t.balign\t4" << std::endl << std::endl;
  return part_entry_.size();
}

int PartData::index() {
  return part_entry_.index();
}

/**
 Create a new Object by peeking at the next three words in the package data.
 \param[in] p package data stream
 \return a new instantiation of a class derived from Object
 */
std::shared_ptr<Object> Object::peek(PackageBytes &p, uint32_t offset)
{
  int pos = p.tell();
  uint32_t header_ = p.get_uint();
  p.get_uint();
  uint32_t class_ = p.get_uint();
  p.seek_set(pos);
  switch (header_ & 0x00000003) {
    case 0:
      if (class_ == 0x00055552)
        return std::make_shared<ObjectSymbol>(offset); // Symbol
      else
        return std::make_shared<ObjectBinary>(offset); // Binary
    case 1: return std::make_shared<ObjectSlotted>(offset); // Array
    case 2: return std::make_shared<ObjectBinary>(offset); // Unknown
    case 3: return std::make_shared<ObjectSlotted>(offset); // Frame
  }
}

int Object::load(PackageBytes &p)
{
  uint32_t header = p.get_uint();
  type_ = (header & 0x00000003);
  if (type_ == 2) {
    std::cout << "ERROR: 0x"
      << std::setw(8) << std::setfill('0') << std::hex << p.tell() << std::dec
      << ": NS Object type unknown." << std::endl;
    size_ = 0;
  }
  flags_ = (header & 0x000000fc);
  if (flags_!=64)
    std::cout << "WARNING: NS Object flags should be 0x40." << std::endl;
  size_  = ((header >> 8) - 8);
  if (size_ < 0) {
    std::cout << "ERROR: NS Object size <0 found." << std::endl;
    size_ = 0;
  }
  ref_cnt_ = p.get_uint();
  class_ = p.get_uint();
  return 0;
}

int Object::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  f << "@ ----- Object" << std::endl;
  f << "obj_" << p.index() << "_" << offset_ << ":" << std::endl;
  f << "\t.int\t(" << size_ << "+8)<<8 | " << flags_ << " | " << type_;
  f << ", " << ref_cnt_;
  switch (type_) {
    case 0:
      f << "\t@ Binary (" << size_ << " bytes)" << std::endl;
      break;
    case 1:
      f << "\t@ Array (" << (size_/4)-1 << " entries)" << std::endl;
      break;
    case 2:
      f << "\t@ WARNING: unknown type (" << size_ << " bytes)" << std::endl;
      break;
    case 3:
      f << "\t@ Frame (" << (size_/4)-1 << " entries)" << std::endl;
      break;
  }
  return 8;
}

int ObjectBinary::load(PackageBytes &p)
{
  Object::load(p);
  data_ = p.get_data(size_-4);
  return 0;
}

int ObjectBinary::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  Object::writeAsm(f, p);
  f << "\t" << p.asmRef(class_) << "\t@ class" << std::endl;
  write_data(f, data_);
  return size_;
}

int ObjectSymbol::load(PackageBytes &p)
{
  Object::load(p);
  hash_ = p.get_uint();
  symbol_ = p.get_cstring(size_-8-1);
  return 0;
}

int ObjectSymbol::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  Object::writeAsm(f, p);
  f << "\t.int\t0x"
  << std::setw(8) << std::setfill('0') << std::hex << class_ << ", 0x" << hash_ << std::dec
  << "\t@ hash" << std::endl;
  f << "\t.asciz\t\"" << symbol_ << "\"" << std::endl;
  return size_;
}

int ObjectSlotted::load(PackageBytes &p)
{
  Object::load(p);
  int i = 0, n = size_/4-1;
  for ( ; i<n; ++i) {
    ref_list_.push_back(p.get_uint());
  }
  return 0;
}

int ObjectSlotted::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  Object::writeAsm(f, p);
  f << "\t" << p.asmRef(class_) << "\t@ class or map" << std::endl;
  for (auto &ref: ref_list_) {
    f << "\t" << p.asmRef(ref) << "\t@ ref" << std::endl;
  }
  return size_;
}

/**
 Read the NOS Part of the Package as a list of Objects.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartDataNOS::load(PackageBytes &p) {
  int start = p.tell();
  int n = start + part_entry_.size();
  
  p.get_uint();
  uint32_t align_bit = p.get_uint();
  if (align_bit & 0x00000001) align_ = 4;
  p.seek_set(start);

  while (p.tell() < n) {
    uint32_t offset = p.tell();
    auto o = Object::peek(p, offset);
    o->load(p);
    object_list_[offset] = o;
    p.align(align_);
  }
  return 0;
}

/**
 Write NOS Package Part data in ARM32 assembler format.
 \param[in] f output stream
 \return number of bytes written
 */
int PartDataNOS::writeAsm(std::ofstream &f) {
  f << "@ ===== Part " << part_entry_.index() << " Data NOS" << std::endl;
  f << "part_" << part_entry_.index() << ":" << std::endl;
  f << std::endl;
  for (auto &obj: object_list_) {
    obj.second->writeAsm(f, *this);
    f << "\t.balign\t" << align_ << ", 0xbf" << std::endl;
  }
  f << "\t.balign\t4" << std::endl << std::endl;
  return part_entry_.size();
}

std::string PartDataNOS::asmRef(uint32_t ref)
{
  static char buf[80];
  switch (ref & 3) {
    case 0: // integer
      ::snprintf(buf, 79, "ref_integer\t%d", ref/4);
      break;
    case 1: // pointer
      ::snprintf(buf, 79, "ref_pointer\tobj_%d_%d", index(), ref & ~3);
      break;
    case 2: // special
      if (ref == 2) {
        ::snprintf(buf, 79, "ref_nil");
      } else if (ref == 0x1a) {
        ::snprintf(buf, 79, "ref_true");
      } else if ((ref & 15) == 10) {
        uint32_t c = ref >> 4;
        if (c>=32 && c<127)
          ::snprintf(buf, 79, "ref_unichar '%c'", (char)c);
        else
          ::snprintf(buf, 79, "ref_unichar %d", c);
      } else {
        ::snprintf(buf, 79, ".int\t0x%08x", ref);
      }
      break;
    case 3: // magic
      ::snprintf(buf, 79, "ref_magic\t%d", ref/4);
      break;
  }
  return std::string(buf);
}

