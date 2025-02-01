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


#ifndef NEWTFMT_PACKAGE_PACKAGE_H
#define NEWTFMT_PACKAGE_PACKAGE_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

#include "relocation_data.h"

namespace pkg {

class PartEntry;
class PackageBytes;

class Package {
  std::string signature_;
  std::string type_;
  uint32_t flags_ {0};
  uint32_t version_ {0};
  uint16_t copyright_start_ {0};
  uint16_t copyright_length_ {0};
  uint16_t name_start_ {0};
  uint16_t name_length_ {0};
  uint32_t size_ {0};
  uint32_t date_ {0};
  uint32_t reserved2_ {0};
  uint32_t reserved3_ {0};
  uint32_t directory_size_ {0};
  uint32_t num_parts_ {0};
  uint32_t vdata_start_ {0};
//  uint32_t info_start_ {0};
  uint32_t info_length_ {0};
  std::vector<std::shared_ptr<PartEntry>> part_;
  std::string copyright_;
  std::string name_;
  std::vector<uint8_t> info_;
  RelocationData relocation_data_;
public:
  Package() = default;
  int load(PackageBytes &p);
  int writeAsm(std::ofstream &f);
};


} // namespace pkg

#endif // NEWTFMT_PACKAGES_PACKAGE_H


