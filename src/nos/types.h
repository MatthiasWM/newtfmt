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


#ifndef NEWTFMT_NOS_TYPES_H
#define NEWTFMT_NOS_TYPES_H

#include <stdint.h>

namespace nos {

using Boolean = bool;
using Integer = long;
using Index = long;
using Size = long;
using Real = double;
using UniChar = char32_t;

using Ptr = void*;
using NativePtr = void*;
using ObjectPtr = class Object*;

using RefArg = const class Ref &;

using NewtonErr = int;

using Coord = int;

struct Rect
{
  Coord  top;
  Coord  left;
  Coord  bottom;
  Coord  right;
};
typedef struct Rect Rect;

Ref MakeInt(Integer i);
Ref MakeChar(UniChar c);
Ref MakeBool(Boolean b);
Ref MakeMagic(Index i);

Boolean IsNil(RefArg r);
Boolean NotNil(RefArg r);
Boolean IsFalse(RefArg r);
Boolean IsTrue(RefArg r);
Boolean IsBinary(RefArg ref);
Boolean IsArray(RefArg ref);
Boolean IsFrame(RefArg ref);
Boolean IsNumber(Ref ref);
Boolean IsPathExpr(RefArg ref);

Ref AllocateArray(RefArg theClass, Size length);

void Print(RefArg);

} // namespace nos

#endif // NEWTFMT_NOS_TYPES_H

