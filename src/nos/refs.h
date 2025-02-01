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

namespace nos {

class Ref {
  union {
    Ptr ptr_;
    Integer integer_;
  } u;
  uint8_t type_;
public:
  Ref();
  Ref(Integer i);
  ~Ref();
  int Print(PrintState &ps) const;
  void AddArraySlot(RefArg value) const;
};

} // namespace nos

#endif // NEWTFMT_NOS_REFS_H

