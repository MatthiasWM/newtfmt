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

class alignas(long)  Object
{
  friend class Ref;

  enum class Tag: uint8_t {
    binary, array, large_binary, frame
  };

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  typedef struct {
    uint8_t slotted_:1;
    uint8_t frame_:1;
    uint8_t free_:1;
    uint8_t marked_:1;
    uint8_t locked_:1;
    uint8_t forward_:1;
    uint8_t read_only_:1;
    uint8_t dirty_:1;
  } TagFlags;

  typedef struct {
    Tag tag_:2;
    uint8_t flags_:6;
  } Tags;
#else
#error "Sorry, not implemented"
#endif

  typedef struct {
    Ref    class_;
    char   *data_;
  } Binary;

  typedef struct {
    Ref    class_;
//    const  IndirectBinaryProcs * procs;
    char   *data_;
  } LargeBinary;

  typedef struct {
    Ref    class_;
    Ref    *slot_;
    uint32_t reserve_;
  } _Array;

  typedef struct {
    Ref    map_;
    Ref    *slot_;
    uint32_t reserve_;
  } _Frame;

  typedef struct {
    Ref    class_;
    Real   value_;
  } _Real;

  typedef struct {
    Ref    class_;
    char   *string_;
    uint32_t hash_;
  } _Symbol;

  union {
    TagFlags f;
    Tags t;
    uint8_t all_flags_;
  };
  uint32_t size_:24;
  uint32_t gc_ { 0 };
  union {
    Binary bin;
    LargeBinary lbo;
    _Array array;
    _Frame frame;
    _Real real;
    _Symbol sym;
  };
//  Object(const Object&) = delete;
//  Object(Object&&) = delete;
//  Object &operator=(const Object&) = delete;
//  Object &operator=(Object&&) = delete;
  // a little trick to get a constexpr strlen()
  static constexpr uint32_t _strlen(const char* str) {
    return *str ? 1 + _strlen(str + 1) : 0;
  }
  static constexpr uint32_t _hash1(const char* str) {
    uint8_t c = (uint8_t)str[0];
    if (c>='a' && c<='z') c -= 32;
    return *str ? c + _hash1(str+1) : 0;
  }
  static constexpr uint32_t _hash(const char* str) {
    return _hash1(str) * 0x9E3779B9;
  }
public:
  Object() : size_(0), gc_(0) { }

  constexpr Object(uint32_t hash, const char *symbol)
  : t { Tag::binary, 0x10 }, size_( _strlen(symbol) ), sym { RefSymbolClass, (char*)symbol, hash }
  { }

  constexpr Object(uint32_t)
  : t { Tag::frame, 0x10 }, size_{ 0 }, frame { 0, 0, 0 }
  { }

  constexpr Object(Tag tag, Ref class_or_map, uint32_t num_slots, const Ref *values)
  : t { tag, 0x10 }, size_{ static_cast<uint32_t>(num_slots*sizeof(Ref)) }, frame { class_or_map, (Ref*)values, 0 }
  { }

  constexpr Object(const char *string);

  constexpr Object(Real value);

  static constexpr Object Array(Ref obj_class, uint32_t num_slots, const Ref *values) {
    return Object(Tag::array, obj_class, num_slots, values);
  }

  static constexpr Object Frame(Ref map, uint32_t num_slots, const Ref *values) {
    return Object(Tag::frame, map, num_slots, values);
  }

  static constexpr Object Symbol(const char *symbol) {
    return Object(_hash(symbol), symbol);
  }

  Index size() const { return size_; }
  uint32_t gc() const { return gc_; }

  constexpr bool IsBinary() const { return (t.tag_ == Tag::binary); }
  constexpr bool IsArray() const { return (t.tag_ == Tag::array); }
  constexpr bool IsFrame() const { return (t.tag_ == Tag::frame); }

  Ref GetSlot(Index i) const;

  int SymbolCompare(const Object *other) const;

  int Print(PrintState &ps) const;

}; // no need for `__attribute__((packed));`, sizeof(Object) is 32 bytes on a 64bit CPU.

int symcmp(const char *a, const char *b);
int SymbolCompare(Ref sym1, Ref sym2);

constexpr Object gSymObjString { Object::Symbol("string") };
constexpr Ref gSymString { gSymObjString };

constexpr Object gSymObjReal { Object::Symbol("real") };
constexpr Ref gSymReal { gSymObjReal };

constexpr Object::Object(const char *string)
: t { Tag::binary, 0x10 }, size_{ _strlen(string) }, bin { gSymString, const_cast<char*>(string) }
{ }

constexpr Object::Object(Real value)
: t { Tag::binary, 0x10 }, size_{ 0 }, real { gSymReal, value }
{ }



} // namespace nos

#endif // NEWTFMT_NOS_OBJECTS_H

