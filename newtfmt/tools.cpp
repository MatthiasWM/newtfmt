

#include "tools.h"

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <locale>
#include <codecvt>


std::string utf16_to_utf8(std::u16string &wstr) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wstr);
}

std::u16string utf8_to_utf16(std::string &str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
}

int write_utf16(std::ofstream &f, std::string &u8str) {
  f << "\t@ \"" << u8str << "\"" << std::endl;
  f << "\t.short\t";
  auto str16 = utf8_to_utf16(u8str);
  for (auto c: str16) {
    if (c=='\'')
      f << "'\\'', ";
    else if (c>=32 && c<127)
      f << "'" << (char)c << "', ";
    else
      f << "0x" << std::setw(4) << std::hex << (uint16_t)c << std::dec << ", ";
  }
  f << "0x0000" << std::endl;
  return ((int)str16.size()+1) * 2;
}

int write_data(std::ofstream &f, std::vector<uint8_t> &data) {
  int i, j, n = (int)data.size();
  for (i = 0; i < n; i+=8) {
    f << "\t.byte\t";
    for (j = 0; j < 8 && i+j < n; j++) {
      if (j>0) f << ", ";
      f << "0x" << std::setw(2) << std::hex << (int)data[i+j] << std::dec;
    }
    f << "\t@ |";
    for (j = 0; j < 8 && i+j < n; j++) {
      uint8_t c = data[i+j];
      if (c>=32 && c<127)
        f << (char)c;
      else
        f << ".";
    }
    f << "|" << std::endl;
  }
  return n;
}
