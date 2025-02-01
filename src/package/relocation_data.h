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


#ifndef NEWTFMT_PACKAGE_RELOCATION_DATA_H
#define NEWTFMT_PACKAGE_RELOCATION_DATA_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

namespace pkg {

class PackageBytes;

class RelocationSet {
  uint16_t page_number_{ 0 };
  uint16_t offset_count_{ 0 };
  std::vector<uint8_t> offset_list_;
  std::vector<uint8_t> padding_;
public:
  RelocationSet() = default;
  int load(PackageBytes &p);
  int writeAsm(std::ofstream &f);
};

class RelocationData {
  uint32_t reserved_ {0};
  uint32_t size_ {0};
  uint32_t page_size_ {0};
  uint32_t num_entries_ {0};
  uint32_t base_address_ {0};
  std::vector<RelocationSet> relocation_set_list_;
  std::vector<uint8_t> padding_;
public:
  RelocationData() = default;
  int load(PackageBytes &p);
  int writeAsm(std::ofstream &f);
};

} // namespace pkg

#endif // NEWTFMT_PACKAGE_RELOCATION_DATA_H

