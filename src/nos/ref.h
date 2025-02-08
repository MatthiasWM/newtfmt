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


#ifndef NEWTFMT_NOS_REFS_H
#define NEWTFMT_NOS_REFS_H

#include "nos/types.h"

#include "nos/print.h"

#include <variant>

namespace nos {

constexpr int kRefTagBits = 2;
constexpr int kRefValueBits = sizeof(uintptr_t) * 8 - kRefTagBits;
constexpr uintptr_t kRefValueMask = (~(uintptr_t)0) << kRefTagBits;
constexpr uintptr_t kRefTagMask = ~kRefValueMask;
constexpr int kRefImmedBits = 2;
constexpr uintptr_t kRefImmedMask = (~(uintptr_t)0) << kRefImmedBits;

constexpr uint32_t kTagPointer  = 0;
constexpr uint32_t kTagInteger  = 1;
constexpr uint32_t kTagImmed    = 2;
constexpr uint32_t kTagMagicPtr = 3;

constexpr int nBits = 30;

#ifndef __BYTE_ORDER__
#error byte order not defined
#endif

class Ref
{
public:
  enum class Tag: uint8_t {
    pointer, integer, immed, magic
  };

  enum class Type: uint8_t {
    special, unichar, boolean, reserved
  };

private:
  typedef struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    Tag tag_:kRefTagBits;
    uintptr_t value_:kRefValueBits;
#else
    uintptr_t value_:kRefValueBits;
    Tag tag_:kRefTagBits;
#endif
  } Value;

  typedef struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    Tag tag_:kRefTagBits;
    Type type_:kRefImmedBits;
    uintptr_t value_:kRefValueBits-kRefImmedBits;
#else
    uintptr_t value_:kRefValueBits-kRefImmedBits;
    Type type_:kRefImmedBits;
    Tag tag_:kRefTagBits;
#endif
  } Immed;

  union {
    uintptr_t t;
    Value v;
    Immed i;
//  Magic table:18, index:12, Tag:2
    Object *o;
  };

public:

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  constexpr Ref() : i { Tag::immed, Type::special, 0 } { };
  constexpr Ref(Type type, Integer ival) : i { Tag::immed, type, (unsigned long)ival } { };
  constexpr Ref(Integer i) : v { Tag::integer, (unsigned long)i } {}
  constexpr Ref(int i) : v { Tag::integer, (unsigned long)i } {}
  constexpr Ref(UniChar u) : i { Tag::immed, Type::unichar, u } {}
#else
  constexpr Ref() : i { 0, Type::special, Tag::immed } { };
  constexpr Ref(Type type, Integer ival) : i { (unsigned long)ival, type, Tag::immed } { };
  constexpr Ref(Integer i) : v { (unsigned long)i, Tag::integer } {}
  constexpr Ref(int i) : v { (unsigned long)i, Tag::integer } {}
  constexpr Ref(UniChar u) : i { u, Type::unichar, Tag::immed } {}
#endif
  constexpr Ref(const Object &obj): o(const_cast<Object*>(&obj)) { }
  constexpr Ref(Object *obj): o(obj) { }

//  constexpr Ref(const Ref &other) { i = other.i; }
//  Ref(Ref &&other);
//  Ref &operator=(const Ref &other);
//  Ref &operator=(Ref &&other);
//  ~Ref() = default;
//  void Assign(Integer i);
//  Boolean IsInteger();
//
//  Ref(Object *o): v { ((unsigned long)o)>>kRefTagBits, Tag::pointer } { }
//  void Assign(ObjectPtr o);
//  Boolean IsObject();
//
//  constexpr Ref(Object &o): ref_(&o) { o.read_only_ = false; o.incr_ref_count(); }
//
//  void AddArraySlot(RefArg value) const;
  constexpr bool operator==(const Ref &other) const { return t == other.t; }

  constexpr bool IsPtr() const { return (v.tag_ == Tag::pointer); }

  bool IsBinary() const;
  bool IsArray() const;
  bool IsFrame() const;

  Ref GetArraySlot(Index slot) const;
  Object *GetObject() const { return IsPtr() ? o : nullptr; }

  int Print(PrintState &ps) const;
};

using RefArg = const Ref&;

constexpr Ref RefNIL;
constexpr Ref RefTRUE { Ref::Type::boolean, 1 };
constexpr Ref RefSMILE { U'😀' };
constexpr Ref RefPYTHON { 42 };
constexpr Ref RefSymbolClass { Ref::Type::special, 0x5555 };

constexpr bool IsPtr(Ref r) { return r.IsPtr(); }
inline bool IsBinary(Ref r) { return r.IsBinary(); }
inline bool IsArray(Ref r) { return r.IsArray(); }
inline bool IsFrame(Ref r) { return r.IsFrame(); }
inline Ref GetArraySlot(RefArg array, Index slot) { return array.GetArraySlot(slot); }

} // namespace nos

#endif // NEWTFMT_NOS_REFS_H

