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
#include <cstring>

using namespace nos;

static_assert(sizeof(Ref)==sizeof(uintptr_t), "'Ref' has unexpected size.");

bool Ref::IsBinary() const {
  return IsPtr() && o->IsBinary();
}

bool Ref::IsArray() const {
  return IsPtr() && o->IsArray();
}

bool Ref::IsFrame() const {
  return IsPtr() && o->IsFrame();
}

bool Ref::IsSymbol() const {
  return IsPtr() && o->IsSymbol();
}

bool Ref::IsReadOnly() const {
  auto obj = GetObject();
  if (obj)
    return obj->IsReadOnly();
  else
    return false;
}

int Ref::Print(PrintState &ps) const
{
  switch (v.tag_) {
    case Tag::pointer:
      o->Print(ps);
      break;
    case Tag::integer:
      fprintf(ps.out_, "%ld", (Integer)v.value_);
      break;
    case Tag::immed:
      switch (i.type_) {
        case Type::unichar:
          fprintf(ps.out_, "$%s", unicode_to_utf8((UniChar)i.value_).c_str());
          break;
        case Type::special:
          if (i.value_==0) {
            fprintf(ps.out_, "NIL");
          } else {
            fprintf(ps.out_, "[undefined special: %ld]", i.value_);
          }
          break;
        case Type::boolean:
          if (i.value_==1) {
            fprintf(ps.out_, "TRUE");
          } else {
            fprintf(ps.out_, "[undefined boolean: %ld]", i.value_);
          }
          break;
        case Type::reserved:
          if (sizeof(t)==4)
            fprintf(ps.out_, "[reserved: 0x%08lx]", t);
          else
            fprintf(ps.out_, "[reserved: 0x%086lx]", t);
          break;
      }
      break;
    case Tag::magic:
      fprintf(ps.out_, "@%ld", v.value_);
      break;
  }

  return 0;
}


