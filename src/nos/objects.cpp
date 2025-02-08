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
//  fprintf(ps.out_, "Object <0x%016lx> {\n", (uintptr_t)this);
//  //  fprintf(ps.out_, "  flags dirty:%d read_only:%d forward:%d locked:%d marked:%d free:%d frame:%d slotted:%d\n",
//  //          f.dirty_, f.read_only_, f.forward_, f.locked_, f.marked_, f.free_, f.frame_, f.slotted_);
//  //  fprintf(ps.out_, "  tag flags:%02x tag:%d\n", t.flags_, (int)t.tag_);
//  fprintf(ps.out_, "  raw flags:%02x\n", all_flags_);
//  fprintf(ps.out_, "  size %ld\n", size());

  switch (t.tag_) {
    case Tag::binary:
      if (bin.class_ == RefSymbolClass) {
        ps.tab();
        fprintf(ps.out_, "'%s\n", sym.string_);
      } else {
        ps.tab();
        fprintf(ps.out_, "binary(%ld)\n", size());
      }
      break;
    case Tag::large_binary:
      ps.tab();
      fprintf(ps.out_, "large_binary(%ld)\n", size());
      break;
    case Tag::array: {
      ps.tab();
      fprintf(ps.out_, "array [\n");
      ps.incr_depth();
      array.class_.Print(ps);
      int i, n = (int)(size()/sizeof(Ref));
      for (i=0; i<n; ++i) {
        array.slot_[i].Print(ps);
      }
      ps.decr_depth();
      ps.tab();
      fprintf(ps.out_, "]\n");
    } break;
    case Tag::frame: {
      ps.tab();
      fprintf(ps.out_, "frame {\n");
      ps.incr_depth();
      int i, n = (int)(size()/sizeof(Ref));
      for (i=0; i<n; ++i) {
        frame.map_.GetArraySlot(i+1).Print(ps);
        GetSlot(i).Print(ps);
      }
      ps.decr_depth();
      ps.tab();
      fprintf(ps.out_, "}\n");
    } break;
  }

//  fprintf(ps.out_, "}\n");

  return 0;
}
