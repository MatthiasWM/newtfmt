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

bool Ref::IsArray() const {
  return IsPtr() && o->IsArray();
}

int Ref::Print(PrintState &ps) const {
  switch (v.tag_) {
    case Tag::pointer:
      o->Print(ps);
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

  return 0;
}
