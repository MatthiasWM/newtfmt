//
//
//

#ifndef NEWTFMT_PACKAGE_BYTES_H
#define NEWTFMT_PACKAGE_BYTES_H

#include <ios>
#include <cstdlib>
#include <vector>

namespace ns {

class PackageBytes : public std::vector<uint8_t>
{
  PackageBytes::iterator it_;

public:
  PackageBytes() = default;
  void rewind();
  int tell();
  bool eof();
  uint8_t get_ubyte();
  uint16_t get_ushort();
  uint32_t get_uint();
  std::string get_cstring(int n, bool trailing_nul=true);
  std::string get_ustring(int n, bool trailing_nul=true);
  std::vector<uint8_t> get_data(int n);
  void align(int n);
};

}; // namespace ns

#endif // NEWTFMT_PACKAGE_BYTES_H

