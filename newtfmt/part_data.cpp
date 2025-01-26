


#include "part_data.h"

#include "package_bytes.h"
#include "part_entry.h"
#include "tools.h"

#include <iostream>
#include <fstream>
#include <ios>

using namespace ns;

/**
 Read the Part of the Package as raw data.
 \param[in] p package data stream
 \return 0 if succeeded
 */
int PartDataGeneric::load(PackageBytes &p) {
  data_ = p.get_data(part_entry_.size());
  return 0;
}

/**
 Write raw Package Part data in ARM32 assembler format.
 \param[in] f output stream
 \return number of bytes written
 */
int PartDataGeneric::writeAsm(std::ofstream &f) {
  f << "@ ===== Part " << part_entry_.index() << " Data Generic" << std::endl;
  write_data(f, data_);
  f << "\t.balign\t4" << std::endl << std::endl;
  return part_entry_.size();
}
