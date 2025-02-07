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


#ifndef NEWTFMT_NOS_OBJECTS_H
#define NEWTFMT_NOS_OBJECTS_H

#include "nos/types.h"

#include "nos/ref.h"

#include <string>
#include <vector>

namespace nos {

class Ref;

class Object {
  //  enum class Tag: uint8_t {
  //    pointer, integer, immed, magic
  //  };
#if 0
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  typedef struct {
    uint8_t dirty_:1;
    uint8_t read_only_:1;
    uint8_t forward_:1;
    uint8_t locked_:1;
    uint8_t marked_:1;
    uint8_t free_:1;
    uint8_t frame_:1;
    uint8_t slotted_:1;
  } Flags;
#else
#endif
  union {
    Flags f;
    unit8_t flags_;
  };
  uint32_t size_:24;
#endif

//  kObjSlotted    = 0x01,
//  kObjFrame    = 0x02,
//  kObjFree      = 0x04,
//  kObjMarked    = 0x08,
//  kObjLocked    = 0x10,
//  kObjForward    = 0x20,
//  kObjReadOnly  = 0x40,
//  kObjDirty    = 0x80,

//  kBinaryObject  = 0x00,
//  kIndirectBinaryObject = 0x02,
//  kArrayObject  = 0x01,
//  kFrameObject  = 0x03,

//  uint32_t size  : 24; \
//  uint32_t flags :  8; \
//  union { \
//    struct { \
//      uint32_t  locks :  8; \
//      uint32_t  slots : 24; \
//    } count; \
//    Ref stuff; \
//    Ref destRef; \
//  }gc;

// Binary Data:
//  Ref    objClass;
//  char   data[];

// Indirect Binary
//  Ref    objClass;
//  const  IndirectBinaryProcs * procs;
//  char   data[];

// Array
//  Ref    objClass;
//  Ref    slot[];
//  (uint32_t reserve;)

// Frame
//  Ref    map;
//  Ref    slot[];
//  (uint32_t reserve map;)
//  (uint32_t reserve slots;)

// Real
//  Ref    objClass;
//  Real   double;

// Symbol
//  Ref    objClass;
//  uint32_t hash;
//  char *string

  friend class Ref;
//  Object(const Object&) = delete;
//  Object(Object&&) = delete;
//  Object &operator=(const Object&) = delete;
//  Object &operator=(Object&&) = delete;
protected:
//  enum class Type: uint8_t {
//    Symbol,
//    Array,
//    Frame,
//    Binary
//  };
//  int ref_count_{ 0 };
//  Type type_;
//  bool read_only_{ true };
//  ~Object() = default;
//  void incr_ref_count();
//  void decr_ref_count();
public:
  Object() { }
//  constexpr Object(Type type) : type_(type) { }
};


//class BinaryObject : public Object {
//  Ref class_;
//  //std::array<uint8_t> data_;
//  //std::vector<uint8_t> data_ { nullptr };
//protected:
//public:
//  ~BinaryObject() = default;
//  constexpr BinaryObject() : Object(Type::Binary) { }
//};

//class RealObject : public Object {
//  Ref class_; // is 'real
//  double value_;
//protected:
//  ~RealObject() = default;
//public:
//  constexpr RealObject() { }
//};

//class SymbolObject : public Object {
//public:
//  uint32_t hash_;
//  const char *symbol_;
//protected:
//public:
//  constexpr SymbolObject(uint32_t hash, const char *sym)
//  : Object(Type::Symbol), hash_(hash), symbol_(sym) { }
//  ~SymbolObject() = default;
//  uint32_t hash() { return hash_; }
//  const char *symbol() { return symbol_; }
//};

//class SlottedObject : public Object {
//protected:
//  ~SlottedObject() = default;
//public:
//  SlottedObject() = default;
//};

//class ArrayObject : public SlottedObject {
//  Ref class_;
//  std::vector<Ref> ref_array_;
//protected:
//  ~ArrayObject() = default;
//public:
//  ArrayObject() = default;
//};

//class FrameObject : public SlottedObject {
//  std::map<Ref, Ref> ref_map_;
//protected:
//  ~FrameObject() = default;
//public:
//  FrameObject() = default;
//};

} // namespace nos

#endif // NEWTFMT_NOS_OBJECTS_H

