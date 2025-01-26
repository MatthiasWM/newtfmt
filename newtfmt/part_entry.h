
#ifndef NEWTFMT_PART_ENTRY_H
#define NEWTFMT_PART_ENTRY_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

namespace ns {

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

} // namespace ns

#endif // NEWTFMT_PART_ENTRY_H

