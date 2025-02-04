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


#include "nos/refs.h"

using namespace nos;

constexpr uint8_t kRefTypeUnref = 0;
constexpr uint8_t kRefTypeInt = 1;


Ref::Ref() {
  type_ = kRefTypeUnref;
  u.ptr_ = nullptr;
}

Ref::Ref(Integer i) {
  type_ = kRefTypeInt;
  u.integer_ = i;
}

Ref::~Ref() {
}

int Ref::Print(PrintState &ps) const {
  switch (type_) {
    case kRefTypeInt:
      fprintf(ps.out_, "%ld\n", u.integer_);
      break;
    default:
      fprintf(ps.out_, "<unknown>\n");
  }
  return 0;
}

Ref nos::MakeInt(Integer i) {
  return Ref(i);
}
