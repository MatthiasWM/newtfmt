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


#ifndef NEWTFMT_PART_ENTRY_H
#define NEWTFMT_PART_ENTRY_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

namespace pkg {

class PartData;
class PackageBytes;

class PartEntry {
  int index_;
  uint32_t offset_ {0};
  uint32_t size_ {0};
  uint32_t size2_ {0};
  std::string type_;
  uint32_t reserved_ {0};
  uint32_t flags_ {0};
  uint16_t info_offset_ {0};
  uint16_t info_length_ {0};
  uint16_t compressor_offset_ {0};
  uint16_t compressor_length_ {0};
  std::string info_;
  std::shared_ptr<PartData> part_data_;
public:
  PartEntry(int ix);
  int size();
  int index();
  int load(PackageBytes &p);
  int loadInfo(PackageBytes &p);
  int loadPartData(PackageBytes &p);
  int writeAsm(std::ofstream &f);
  int writeAsmInfo(std::ofstream &f);
  int writeAsmPartData(std::ofstream &f);
};

} // namespace pkg

#endif // NEWTFMT_PART_ENTRY_H

