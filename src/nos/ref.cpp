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


#include "nos/ref.h"

#include "nos/objects.h"
#include "tools/tools.h"

#include <iostream>

using namespace nos;

static_assert(sizeof(Ref)==sizeof(uintptr_t), "'Ref' has unexpected size.");


#if 0
/**
 Default constructor.
 */
//Ref::Ref() {
//  ref_ = false;
//}

/**
 Copy constructor.
 */
Ref::Ref(const Ref &other) {
  ref_ = other.ref_;
  if (auto o = std::get_if<Object*>(&ref_))
    (*o)->incr_ref_count();
}

/**
 Move constructor.
 */
Ref::Ref(Ref &&other) {
  ref_ = other.ref_;
  other.ref_ = false;
}

/**
 Assignment operator.
 */
Ref &Ref::operator=(const Ref &other) {
  ref_ = other.ref_;
  if (auto o = std::get_if<Object*>(&ref_))
    (*o)->incr_ref_count();
  return *this;
}

/**
 Move assignment operator.
 */
Ref &Ref::operator=(Ref &&other) {
  ref_ = other.ref_;
  other.ref_ = false;
  return *this;
}

void Ref::Assign(Integer i) {
  unref();
  ref_ = i;
}

Boolean Ref::IsInteger() {
  return std::holds_alternative<Integer>(ref_);
}

/**
 Clear Ref for a new value, decrementing ref counts if pointing to an objects.
 */
void Ref::unref() {
  if (auto o = std::get_if<Object*>(&ref_))
    (*o)->decr_ref_count();
  ref_ = false;
}

//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
//template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
//  std::visit(overloaded{
//    [](auto arg) { std::cout << arg << ' '; },
//    [ps](Integer arg) { fprintf(ps.out_, "int: %ld\n", arg); }
//  }, ref_);


Ref nos::MakeInt(Integer i) {
  return Ref(i);
}

#endif

int Ref::Print(PrintState &ps) const {
  // switch (ref_.index()) { ...
  // v1.emplace<std::string>("def");
  // var.valueless_by_exception()  and index std::variant_npos
  // std::variant<std::monostate, S> (default constructor, unreferenced)
  switch (v.tag_) {
    case Tag::pointer:
      fprintf(ps.out_, "<0x%016lx>\n", v.value_ << 2);
      break;
    case Tag::integer:
      fprintf(ps.out_, "%ld\n", (Integer)v.value_);
      break;
    case Tag::immed:
      switch (i.type_) {
        case Type::unichar:
          fprintf(ps.out_, "'%s\n", unicode_to_utf8((UniChar)i.value_).c_str());
          break;
        case Type::special:
          if (i.value_==0) {
            fprintf(ps.out_, "NIL\n");
          } else {
            fprintf(ps.out_, "[undefined special: %ld]\n", i.value_);
          }
          break;
        case Type::boolean:
          if (i.value_==1) {
            fprintf(ps.out_, "TRUE\n");
          } else {
            fprintf(ps.out_, "[undefined boolean: %ld]\n", i.value_);
          }
          break;
        case Type::reserved:
          fprintf(ps.out_, "[reserevd]\n");
          break;
      }
      break;
    case Tag::magic:
      fprintf(ps.out_, "[magic]\n");
      break;
  }

//
//  if (auto v = std::get_if<Integer>(&ref_)) {
//    fprintf(ps.out_, "%ld\n", *v);
//    // } else if (std::holds_alternative<Real>(ref_) {...
//  } else if (auto v = std::get_if<const Object*>(&ref_)) {
//    const Object *obj = *v;
//    fprintf(ps.out_, "const Object* 0x%016lx [%d,%d]: ", (uintptr_t)*v, obj->ref_count_, obj->read_only_);
//  } else if (auto v = std::get_if<Object*>(&ref_)) {
//    Object *obj = *v;
//    fprintf(ps.out_, "Object* 0x%016lx [%d,%d]: ", (uintptr_t)*v, obj->ref_count_, obj->read_only_);
//  } else {
//    fprintf(ps.out_, "<unknown>\n");
//  }
  return 0;
}
