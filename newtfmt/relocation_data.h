
#ifndef NEWTFMT_RELOCATION_DATA_H
#define NEWTFMT_RELOCATION_DATA_H

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>

namespace ns {

class PackageBytes;

// TODO: Relocation Set
//      repeat num_entries_
//      .balign 4
//      struct RelocationSet {
//        UShort pageNumber;
//        UShort offsetCount;
//        /* Byte offsets[]; */
//      };

class RelocationData {
  uint32_t reserved_ {0};
  uint32_t size_ {0};
  uint32_t page_size_ {0};
  uint32_t num_entries_ {0};
  uint32_t base_address_ {0};
  std::vector<uint8_t> data_;
public:
  RelocationData() = default;
  int load(PackageBytes &p);
  int writeAsm(std::ofstream &f);
};

} // namespace ns

#endif

