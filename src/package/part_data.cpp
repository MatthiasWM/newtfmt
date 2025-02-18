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


#include "part_data.h"

#include "package_bytes.h"
#include "part_entry.h"
#include "tools/tools.h"

#include <iostream>
#include <fstream>
#include <ios>

using namespace pkg;

/** \class pkg::PartData
 Base class for holding the data in a NewtonScript Package Part
 */

/**
 Index of this Part in the list of Parts in the Package.
 \return 0-based index
 */
int PartData::index() {
  return part_entry_.index();
}

/**
 Compare this part with the other part.
 \param[in] other the other part
 \return 0 if they are the same.
 */
int PartData::compare(PartData &other)
{
  std::cout << "WARNING: Part type not supported;" << std::endl;
  (void)other;
  return -1;
}


/** \class pkg::PartDataGeneric
 Holds the uninterpreted data of a Part with raw data or unknown type.
 */

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
  f << "@ ===== Part " << part_entry_.index() << " End" << std::endl << std::endl;
  return part_entry_.size();
}


/** \class pkg::Object
 Any kind of Object from the Newton Object System.
 */

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
      // TODO: use the symbol to get information and find Reals and ByteCode
      // There are also machine code block, bitmaps, sounds etc. .
      if (class_ == 0x00055552)
        return std::make_shared<ObjectSymbol>(offset); // Symbol
      else
        return std::make_shared<ObjectBinary>(offset); // Binary
    case 1:
      // If the class is an integer, the array is used to store a map
      // for a Frame. Check what flags are set (sorted(1), _proto(4)),
      // and if any map has a supermap.
      if ((class_ & 0x00000003) == 0)
        return std::make_shared<ObjectMap>(offset); // Map
      else
        return std::make_shared<ObjectSlotted>(offset); // Array
      // TODO: what other special class values are there?
    default:
    case 2: return std::make_shared<ObjectBinary>(offset); // Unknown
    case 3: return std::make_shared<ObjectSlotted>(offset); // Frame
  }
}

/**
 Load the head of an Object; derived class must load the remaining data.
 \parm[in] p reference to the biary data
 \return 0 if successful
 */
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
  //  kObjFree      = 0x04,
  //  kObjMarked    = 0x08,
  //  kObjLocked    = 0x10,
  //  kObjForward   = 0x20,
  //  kObjReadOnly  = 0x40,
  //  kObjDirty     = 0x80 can be set in some cases
  flags_ = (header & 0x000000fc);
  if (flags_!=64) // Read Only
    std::cout << "WARNING: NS Object flags should be 0x40, but it's 0x"
    << std::setw(2) << std::setfill('0') << std::hex << (header & 0x000000fc) << std::dec
    << "." << std::endl;
  size_  = ((header >> 8) - 8);
  if (size_ < 0) {
    std::cout << "ERROR: NS Object size <0 found." << std::endl;
    size_ = 0;
  }
  ref_cnt_ = p.get_uint();
  class_ = p.get_ref();
  return 0;
}

/**
 If the data is unaligned, load the filler bytes.
 \param[in] p raw package data stream
 \param[in] start position of first byte of the current part in the package data
 \param[in] align align to 4 bytes (NewtonOS 2.x) or 8 bytes (NewtonOS 1.x)
 */
void Object::loadPadding(PackageBytes &p, uint32_t start, uint32_t align) {
  align--;
  uint32_t fpos = p.tell() - start;
  uint32_t apos = (fpos + align) & ~align;
  uint32_t n = apos - fpos;
  if (n) padding_ = p.get_data(n);
}

/**
 Write the common header data of an NOS object.
 The caption and the actual data is written by the derived class.
 \param[in] f assembler file output stream
 \param[in] p back reference to part data
 \return number of bytes written
 */
int Object::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  (void)p;
  f << label() << ":" << std::endl;
  f << "\t.int\t(" << size_ << "+8)<<8 | " << flags_ << " | " << type_;
  f << ", " << ref_cnt_ << std::endl;
  return 8;
}

/**
 Generate a simple assembler label using the part index and the offset in the package file.
 \param[in] p Part data reference.
 */
void Object::makeAsmLabel(PartDataNOS &p) {
  char buf[32];
  ::snprintf(buf, 31, "obj_%d_%d", p.index(), offset_);
  label_ = buf;
}

/** \class pkg::Object
 A Binary Object from the Newton Object System.
 */

/**
 Read a binary object form the Package stream.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int ObjectBinary::load(PackageBytes &p)
{
  Object::load(p);
  data_ = p.get_data(size_-4);
  return 0;
}

/**
 Write a binary object in assembler code.
 We could look at the Class entry of the object to find the actual type and
 write the data in a better understandable format. For example, this is
 used for storing floating point data (class = 'real), so we could instead
 just print one FP value.
 \param[in] f assembler file output stream
 \param[in] p back reference to part data
 \return number of bytes written
 */
int ObjectBinary::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  f << "@ ----- " << offset_ << " Binary Object (" << size_-4 << " bytes)" << std::endl;
  Object::writeAsm(f, p);
  f << "\t" << p.asmRef(class_) << "\t@ class" << std::endl;
  write_data(f, data_);
  return size_;
}

/** \class pkg::ObjectSymbol
 A Symbol from the Newton Object System.
 */

/**
 Read a symbol form the Package stream.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int ObjectSymbol::load(PackageBytes &p)
{
  Object::load(p);
  hash_ = p.get_uint();
  symbol_ = p.get_cstring(size_-8-1);
#if 0
  uint32_t fpos = p.tell();
  uint32_t apos = (fpos + 7) & ~7;
  uint32_t n = apos - fpos;
  if (n) {
    printf("align at 0x%08x, %d: ", fpos, n);
    for (int i=0; i<n; i++) {
      printf("%02x", p.get_ubyte());
    }
    printf("\n");
  }
  p.seek_set(fpos);
#endif
  return 0;
}

/**
 Write a Symbol in assembler code.
 \param[in] f assembler file output stream
 \param[in] p back reference to part data
 \return number of bytes written
 */
int ObjectSymbol::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  static char hex[] = "0123456789ABCDEF";
  f << "@ ----- " << offset_ << " Symbol (" << size_-9 << " chars)" << std::endl;
  Object::writeAsm(f, p);
  f << "\t.int\t0x"
  << std::setw(8) << std::setfill('0') << std::hex << class_ << ", 0x" << hash_ << std::dec
  << "\t@ hash" << std::endl;

  std::string ascii_symbol;
  for (auto c: symbol_) {
    if (::isprint(c)) {
      ascii_symbol += c;
    } else {
      uint8_t h = (uint8_t)c;
      ascii_symbol += "\\x";
      ascii_symbol += hex[h>>4];
      ascii_symbol += hex[h&0x0f];
    }
  }

  f << "\t.asciz\t\"" << ascii_symbol << "\"" << std::endl;
  return size_;
}

/**
 Create an assembler label for this symbol.

 Symbols can contain characters that are not legal for labels, so we replace
 those with their hex value. We then check the generated label, and if there is
 already the same label in the file, generate a new label by adding and then
 incrementing a counter at the end of the label.

 The label is then stored with the symbol.

 \param[in] p back reference to part data
 */
void ObjectSymbol::makeAsmLabel(PartDataNOS &p) {
  static char hex[] = "0123456789ABCDEF";
  char buf[128];
//  ::snprintf(buf, 31, "sym_%d_%s", p.index(), label_.c_str());
  ::snprintf(buf, sizeof(buf)-1, "sym_%d_", p.index());
  label_ = buf;
  std::string ascii_label;
  for (auto c: symbol_) {
    if ( ::isalnum(c) || (c=='_') ) {
      label_ += c;
    } else {
      uint8_t h = (uint8_t)c;
      label_ += hex[h>>4];
      label_ += hex[h&0x0f];
    }
  }
  if (p.addLabel(label_, this)==false) {
    strncpy(buf, label_.c_str(), sizeof(buf)-7);
    int ins = (int)strlen(buf);
    for (int i=2; ; i++) {
      snprintf(buf+ins, 6, "_%d", i);
      label_ = buf;
      if (p.addLabel(label_, this)) break;
    }
  }
}

/** \class pkg::ObjectSlotted
 A Slotted Object (Frame or Array) from the Newton Object System.
 */

/**
 Read a slotted object (Frame or Array) form the Package stream.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int ObjectSlotted::load(PackageBytes &p)
{
  Object::load(p);
  int i = 0, n = size_/4-1;
  for ( ; i<n; ++i) {
    ref_list_.push_back(p.get_ref());
  }
  return 0;
}

/**
 Write a slotted object (a Form or an Array) in assembler code.
 \param[in] f assembler file output stream
 \param[in] p back reference to part data
 \return number of bytes written
 */
int ObjectSlotted::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  if (type_ == 1) {
    f << "@ ----- " << offset_ << " Array (" << (size_/4)-1 << " entries)" << std::endl;
    Object::writeAsm(f, p);
    f << "\t" << p.asmRef(class_) << "\t@ class" << std::endl;
  } else {
    f << "@ ----- " << offset_ << " Frame (" << (size_/4)-1 << " entries)" << std::endl;
    Object::writeAsm(f, p);
    f << "\t" << p.asmRef(class_) << "\t@ map" << std::endl;
  }
  for (auto &ref: ref_list_) {
    f << "\t" << p.asmRef(ref) << "\t@ ref" << std::endl;
  }
  return size_;
}

/** \class pkg::ObjectMap
 An Array of Symbols, as used by the Frame Object to create named indexing.
 */

/**
 Write a Frame lookup map object in assembler code.
 \param[in] f assembler file output stream
 \param[in] p back reference to part data
 \return number of bytes written
 */
int ObjectMap::writeAsm(std::ofstream &f, PartDataNOS &p)
{
  f << "@ ----- " << offset_ << " Map (" << (size_/4)-2 << " entries)" << std::endl;
  Object::writeAsm(f, p);
  f << "\t" << p.asmRef(class_) << "\t@ flags" << std::endl;
  // Flags can be 1 (kMapSorted), 2(kMapShared), 4 (kMapProto)
  if (((class_>>2) & ~(1+2+4)) != 0)
    std::cout << "WARNING: Unknown map flag set: " << (class_>>2) << std::endl;
  if (ref_list_.size() > 0) {
    f << "\t" << p.asmRef(ref_list_[0]) << "\t@ supermap";
// Yes, we use supermaps in Packages which will make life slightly harder
//    if (ref_list_[0] != 0x00000002) {
//      std::cout << "WARNING: map references a supermap!" << std::endl;
//      f << " to SUPERMAP";
//    }
    f << std::endl;
    int i, n = (int)ref_list_.size();
    for (i=1; i<n; ++i) {
      f << "\t" << p.asmRef(ref_list_[i]) << "\t@ ref" << std::endl;
    }
  }
  return size_;
}

/** \class pkg::PartDataNOS
 All the data in a NOS Part of the Package.
 */

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
  if (align_bit & 0x00000001) {
    align_ = 4;
    align_fill_ = 0xbfbfbfbf;
  }
  p.seek_set(start);

  while (p.tell() < n) {
    uint32_t offset = p.tell();
    auto o = Object::peek(p, offset);
    o->load(p);
    object_list_[offset] = o;
    o->loadPadding(p, start, align_);
  }

  for (auto &obj: object_list_) {
    obj.second->makeAsmLabel(*this);
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
//    if (align_ == 4) {
//      f << "\t.balign\t" << align_ << ", 0xbf" << std::endl;
//    } else {
      write_data(f, obj.second->padding_);
//    }
  }
  f << "\t.balign\t4" << std::endl << std::endl;
  f << "@ ===== Part " << part_entry_.index() << " End" << std::endl << std::endl;
  return part_entry_.size();
}

/**
 Return the start of an assembler line that will produce the given Ref.
 \param[in] ref a valid Ref
 \return[in] a temporary string with the assembler code
 */
std::string PartDataNOS::asmRef(uint32_t ref)
{
  static char buf[80];
  switch (ref & 3) {
    case 0: // integer
      ::snprintf(buf, 79, "ref_integer\t%d", ref/4);
      break;
    case 1: // pointer
//      ::snprintf(buf, 79, "ref_pointer\tobj_%d_%d", index(), ref & ~3);
      if (object_list_.find(ref&~3) != object_list_.end()) {
        ::snprintf(buf, 79, "ref_pointer\t%s", object_list_[ref&~3]->label().c_str());
      } else {
        std::cout << "WARNING: Invalid reference to offset " << (ref&~3) << "." << std::endl;
        ::snprintf(buf, 79, "ref_pointer_invalid\t0x%08x", ref);
      }
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

/**
 Try to add a label to the list of unique labels.

 This is used to make labels unique within the part. The method is called
 with a suggestion for a label text. If the label is taken, it will return
 false, giving the caller the opportunity to try again with a modified label,
 for example by adding a number that increments until the label is unique.
 If the label is not yet taken, it is added to the list and true is retuned.

 \note The label text and the symbol text may be different to match requirements
 of the assembler and to make the unique if the same symbol exists
 more than once.

 \param[in] label ASCII string
 \param[in] symbol if the label is unique, it will reference this symbol
 */
bool PartDataNOS::addLabel(std::string label, ObjectSymbol *symbol) {
  if (label_list_.find(label) == label_list_.end()) {
    // not found, add the label
    label_list_[label] = symbol;
    return true;
  } else {
    // already in list, caller may try again with another label
    return false;
  }
}

/**
 Compare this NOS part with the other NOS part.
 \param[in] other the other part which must be NOS as well
 \return 0 if they are the same.
 */
int PartDataNOS::compare(PartData &other)
{
  // TODO: write this
  (void)other;
  return -1;
}
