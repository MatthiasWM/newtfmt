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

int Object::Print(PrintState &ps) const
{
  fprintf(ps.out_, "<0x%016lx>\n", (uintptr_t)this);
  fprintf(ps.out_, "Object {\n");
  fprintf(ps.out_, "  flags dirty:%d read_only:%d forward:%d locked:%d marked:%d free:%d frame:%d slotted:%d\n",
          f.dirty_, f.read_only_, f.forward_, f.locked_, f.marked_, f.free_, f.frame_, f.slotted_);
  fprintf(ps.out_, "  tag flags:%02x tag:%d\n", t.flags_, (int)t.tag_);
  fprintf(ps.out_, "  raw flags:%02x\n", all_flags_);
  fprintf(ps.out_, "  size %d\n", size());
  fprintf(ps.out_, "}\n");

  return 0;
}
