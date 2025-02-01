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


#ifndef NEWTFMT_PACKAGE_BYTES_H
#define NEWTFMT_PACKAGE_BYTES_H

#include <ios>
#include <cstdlib>
#include <vector>

namespace pkg {

class PackageBytes : public std::vector<uint8_t>
{
  PackageBytes::iterator it_;

public:
  PackageBytes() = default;
  void rewind();
  void seek_set(int ix);
  int tell();
  bool eof();
  uint8_t get_ubyte();
  uint16_t get_ushort();
  uint32_t get_uint();
  uint32_t get_ref();
  std::string get_cstring(int n, bool trailing_nul=true);
  std::string get_ustring(int n, bool trailing_nul=true);
  std::vector<uint8_t> get_data(int n);
  void align(int n);
};

}; // namespace pkg

#endif // NEWTFMT_PACKAGE_BYTES_H

