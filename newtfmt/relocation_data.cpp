


#include "relocation_data.h"

#include "package_bytes.h"
#include "tools.h"

using namespace ns;


int RelocationSet::load(PackageBytes &p)
{
  page_number_ = p.get_ushort();
  offset_count_ = p.get_ushort();
  offset_list_ = p.get_data(offset_count_);

  uint32_t fpos = p.tell();
  uint32_t apos = (fpos + 3) & ~3;
  uint32_t n = apos - fpos;
  padding_ = p.get_data(n);
  return 0;
}

int RelocationSet::writeAsm(std::ofstream &f)
{
  f << "@ ----- Relocation Set" << std::endl;
  f << "\t.short\t" << (int)page_number_ << "\t@ page_number" << std::endl;
  f << "\t.short\t" << (int)offset_count_ << "\t@ offset_count_" << std::endl;
  for (auto o: offset_list_) {
    int offset_in_part_data = o*4 + page_number_*1024;
    f << "\t.byte\t" << (int)o << "\t@ relocate word at " << offset_in_part_data << std::endl;
  }
  write_data(f, padding_);
  f << std::endl;
  return (int)(4 + offset_list_.size() + padding_.size());
}


/**
 Read relocation data from the package.
 \todo We are not interpreting the Relocation Sets yet and just save the binary
 data. It will be interesting to see how many Packages use this feature.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int RelocationData::load(PackageBytes &p) {
  int start = p.tell();
  reserved_ = p.get_uint();
  size_ = p.get_uint();
  page_size_ = p.get_uint();
  num_entries_ = p.get_uint();
  base_address_ = p.get_uint();
  for (int i=0; i<num_entries_; ++i) {
    relocation_set_list_.push_back(RelocationSet());
    int result = relocation_set_list_[i].load(p);
    if (result != 0)
      return -1;
  }
  int pading_size_ = start + size_ - p.tell();
  if (pading_size_ > 0) {
    padding_ = p.get_data(pading_size_);
  } else if (pading_size_ < 0) {
    std::cout << "ERROR: Relocation Data padding is negative." << std::endl;
    return -1;
  }
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
  for (auto &set: relocation_set_list_) {
    set.writeAsm(f);
  }
  write_data(f, padding_);
  f << std::endl;
  return size_;
}


