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

class BinaryObject : public Object {
  Ref class_;
  //std::array<uint8_t> data_;
  //std::vector<uint8_t> data_ { nullptr };
protected:
  ~BinaryObject() = default;
public:
  constexpr BinaryObject() : Object(Type::Binary) { }
};

//class RealObject : public Object {
//  Ref class_; // is 'real
//  double value_;
//protected:
//  ~RealObject() = default;
//public:
//  constexpr RealObject() { }
//};

class SymbolObject : public Object {
public:
  uint32_t hash_;
  const char *symbol_;
protected:
public:
  constexpr SymbolObject(uint32_t hash, const char *sym)
  : Object(Type::Symbol), hash_(hash), symbol_(sym) { }
  ~SymbolObject() = default;
  uint32_t hash() { return hash_; }
  const char *symbol() { return symbol_; }
};

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

