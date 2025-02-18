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


#include "nos/objects.h"

using namespace nos;

Ref Object::GetSlot(Index i) const {
  if ((t.tag_==Tag::array) || (t.tag_==Tag::frame)) {
    if (i<(Index)(size()/sizeof(Ref)))
      return frame.slot_[i];
    else
      return RefNIL;
  } else {
    return RefNIL;
  }
}

int nos::symcmp(const char *s1, const char *s2)
{
  for (;;) {
    unsigned char c1 = static_cast<unsigned char>(*s1++);
    unsigned char c2 = static_cast<unsigned char>(*s2++);
    if (c1 == 0) {
      if (c2 == 0)
        return 0;
      else
        return -1;
    }
    if (c2 == 0)
      return 1;
    c1 = std::tolower(c1);
    c2 = std::tolower(c2);
    if (c1 != c2) {
      if (c1 > c2)
        return 1;
      else
        return -1;
    }
  }
}

int Object::SymbolCompare(const Object *other) const
{
  if (symbol.hash_ != other->symbol.hash_) {
    if (symbol.hash_ > other->symbol.hash_)
      return 1;
    else
      return -1;
  }
  return symcmp(symbol.string_, other->symbol.string_);
}

int SymbolCompare(Ref sym1, Ref sym2)
{
  if (sym1 == sym2)
    return 0;
  Object *obj1 = sym1.GetObject();
  Object *obj2 = sym2.GetObject();
  return obj1->SymbolCompare(obj2);
}

int Object::Print(PrintState &ps) const
{
  switch (t.tag_) {
    case Tag::binary:
      if (binary.class_.GetObject()->SymbolCompare(&gSymObjString)==0) {
        fprintf(ps.out_, "\"%s\"", binary.data_); // TODO: must escape characters, is \0 always at the end?
      } else {
        //'samples, 'instructions, 'code, 'bits, 'mask, 'cbits etc.
        fprintf(ps.out_, "binary(");
        ps.expect_symbol(true);
        binary.class_.Print(ps);
        ps.expect_symbol(false);
        fprintf(ps.out_, ": <%ld bytes>)", size());
      }
      break;
    case Tag::large_binary:
      fprintf(ps.out_, "large_binary('");
      ps.expect_symbol(true);
      binary.class_.Print(ps);
      ps.expect_symbol(false);
      fprintf(ps.out_, ": <%ld bytes>)", size());
      break;
    case Tag::array: {
      fprintf(ps.out_, "[\n");
      ps.incr_depth();
      ps.tab();
      ps.expect_symbol(true);
      array.class_.Print(ps);
      ps.expect_symbol(false);
      fprintf(ps.out_, ":\n");
      int i, n = (int)(size()/sizeof(Ref));
      for (i=0; i<n; ++i) {
        ps.tab();
        array.slot_[i].Print(ps);
        if (i+1<n) fprintf(ps.out_, ",");
        fprintf(ps.out_, "\n");
      }
      ps.decr_depth();
      ps.tab();
      fprintf(ps.out_, "]");
    } break;
    case Tag::frame: {
      fprintf(ps.out_, "{\n");
      ps.incr_depth();
      int i, n = (int)(size()/sizeof(Ref));
      for (i=0; i<n; ++i) {
        ps.tab();
        ps.expect_symbol(true);
        frame.map_.GetArraySlot(i+1).Print(ps);
        ps.expect_symbol(false);
        fprintf(ps.out_, ": ");
        GetSlot(i).Print(ps);
        if (i+1<n) fprintf(ps.out_, ",");
        fprintf(ps.out_, "\n");
      }
      ps.decr_depth();
      ps.tab();
      fprintf(ps.out_, "}");
    } break;
    case Tag::real:
      fprintf(ps.out_, "%g", real.value_);
      break;
    case Tag::symbol:
      if (!ps.symbol_expected())
        fprintf(ps.out_, "'");
      fprintf(ps.out_, "%s", symbol.string_);
      break;
    case Tag::native_ptr:
      fprintf(ps.out_, "<NativePtr>");
      break;
    case Tag::reserved:
      fprintf(ps.out_, "<Reserved>");
      break;
  }
  return 0;
}
