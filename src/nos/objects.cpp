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
  if (f.slotted_) {
    if (i<(Index)(size()/sizeof(Ref)))
      return frame.slot_[i];
    else
      return RefNIL;
  } else {
    return RefNIL;
  }
}

int Object::Print(PrintState &ps) const
{
  switch (t.tag_) {
    case Tag::binary:
      if (bin.class_ == RefSymbolClass) {
        if (!ps.symbol_expected())
          fprintf(ps.out_, "'");
        fprintf(ps.out_, "%s", sym.string_);
      } else {
        fprintf(ps.out_, "binary(");
        ps.expect_symbol(true);
        bin.class_.Print(ps);
        ps.expect_symbol(false);
        fprintf(ps.out_, ": <%ld bytes>)", size());
      }
      break;
    case Tag::large_binary:
      fprintf(ps.out_, "large_binary('");
      ps.expect_symbol(true);
      bin.class_.Print(ps);
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
        if (i<n) fprintf(ps.out_, ",");
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
        if (i<n) fprintf(ps.out_, ",");
        fprintf(ps.out_, "\n");
      }
      ps.decr_depth();
      ps.tab();
      fprintf(ps.out_, "}");
    } break;
  }
  return 0;
}
