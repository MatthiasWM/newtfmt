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

class Object {
  friend class Ref;
  Object(const Object&) = delete;
  Object(Object&&) = delete;
  Object &operator=(const Object&) = delete;
  Object &operator=(Object&&) = delete;
protected:
  enum class Type: uint8_t {
    Symbol,
    Array,
    Frame,
    Binary
  };
  int ref_count_{ 0 };
  Type type_;
  bool read_only_{ true };
  ~Object() = default;
  void incr_ref_count();
  void decr_ref_count();
public:
  constexpr Object(Type type) : type_(type) { }
};

class Ref {
  // Special, Magic, Char, String
  std::variant<Integer, Real, Boolean, const Object*, Object*> ref_;
  void unref();
public:
  constexpr Ref() : ref_(false) { };
  Ref(const Ref &other);
  Ref(Ref &&other);
  Ref &operator=(const Ref &other);
  Ref &operator=(Ref &&other);
  ~Ref() = default;

  constexpr Ref(Integer i) : ref_(i) {}
  void Assign(Integer i);
  Boolean IsInteger();

  constexpr Ref(const Object &o): ref_(&o) { }
  void Assign(ObjectPtr o);
  Boolean IsObject();

  constexpr Ref(Object &o): ref_(&o) { o.read_only_ = false; o.incr_ref_count(); }

  void AddArraySlot(RefArg value) const;

  int Print(PrintState &ps) const;
};

} // namespace nos

#endif // NEWTFMT_NOS_REFS_H

