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
#include <stdint.h>
//#include <limits>

namespace nos {

class PrintState {
public:
//  prettyPrint: true,
//  printDepth: 3,
//  printLength: nil,
//  uint32_t print_length_{ std::numeric_limits<uint32_t>::max() };
  uint32_t print_depth_{ 3 };
  uint32_t current_depth_{ 0 };
  FILE *out_{ nullptr };
  bool sym_next_{ false };
public:
  PrintState(FILE *fout);
  ~PrintState();
  void tab();
  bool more_depth(); // TODO: bad naming
  bool incr_depth(); // TODO: return value not used
  void decr_depth();
  void expect_symbol(bool s) { sym_next_ = s; }
  bool symbol_expected() { return sym_next_; }
};

} // namespace nos

#endif // NEWTFMT_NOS_PRINT_H

