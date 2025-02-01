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


#ifndef NEWTFMT_NOS_PRINT_H
#define NEWTFMT_NOS_PRINT_H

#include "nos/types.h"

#include <stdio.h>

namespace nos {

class PrintState {
public:
//  uint32_t indent_{ 0 };
  FILE *out_{ nullptr };
public:
  PrintState(FILE *fout);
  ~PrintState();
};

} // namespace nos

#endif // NEWTFMT_NOS_PRINT_H

