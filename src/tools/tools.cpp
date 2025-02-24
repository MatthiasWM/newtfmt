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


#include "tools.h"

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <locale>
#include <codecvt>
#include <memory>
#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

/**
 Convert a Unicode UTF-16 string into UTF-8 byte sequence.
 */
std::string utf16_to_utf8(std::u16string &wstr) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wstr);
}

/**
 Convert a Unicode UTF-8 string into UTF-16 word sequence.
 */
std::u16string utf8_to_utf16(std::string &str) {
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
}

#pragma clang diagnostic pop

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

std::string unicode_to_utf8(char32_t code) {
  if (code <= 0x7F) {
    return std::string{ (char)code };
  }
  if (code <= 0x7FF) {
    return std::string{
      (char)(0xC0 | (code >> 6)),
      (char)(0x80 | (code & 0x3F))
    };
  }
  if (code <= 0xFFFF) {
    return std::string{
      (char)(0xE0 | (code >> 12)),
      (char)(0x80 | ((code >> 6) & 0x3F)),
      (char)(0x80 | (code & 0x3F))
    };
  }
  if (code <= 0x10FFFF) {
    return std::string{
      (char)(0xF0 | (code >> 18)),
      (char)(0x80 | ((code >> 12) & 0x3F)),
      (char)(0x80 | ((code >> 6) & 0x3F)),
      (char)(0x80 | (code & 0x3F))
    };
  }
  return std::string("");
}
