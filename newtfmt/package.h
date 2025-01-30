

#ifndef NEWTFMT_PACKAGE_H
#define NEWTFMT_PACKAGE_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

#include "relocation_data.h"

namespace ns {

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
  uint32_t info_start_ {0};
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


} // namespace ns

#endif // NEWTFMT_PACKAGE_H


