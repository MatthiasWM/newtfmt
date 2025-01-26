


#include "relocation_data.h"

#include "package_bytes.h"
#include "tools.h"

using namespace ns;

/**
 Read relocation data from the package.
 \todo We are not interpreting the Relocation Sets yet and just save the binary
 data. It will be interesting to see how many Packages use this feature.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int RelocationData::load(PackageBytes &p) {
  reserved_ = p.get_uint();
  size_ = p.get_uint();
  page_size_ = p.get_uint();
  num_entries_ = p.get_uint();
  base_address_ = p.get_uint();
  data_ = p.get_data(size_-20);
  return 0;
}

/**
 Write relocation data in ARM32 assembler format.
 \todo The output is not yet symbolic. Absolute values are used
 \param[in] f output stream
 \return number of bytes written
 */
int RelocationData::writeAsm(std::ofstream &f) {
  f << "@ ===== Relocation Data" << std::endl;
  f << "\t.int\t" << reserved_ << "\t@ reserved" << std::endl;
  f << "\t.int\t" << size_ << "\t@ size" << std::endl;
  f << "\t.int\t" << page_size_ << "\t@ page_size" << std::endl;
  f << "\t.int\t" << num_entries_ << "\t@ num_entries" << std::endl;
  f << "\t.int\t" << base_address_ << "\t@ base_address" << std::endl;
  write_data(f, data_);
  f << std::endl;
  return size_;
}


