

#ifndef NEWTFMT_TOOLS_H
#define NEWTFMT_TOOLS_H

#include <string>

std::string utf16_to_utf8(std::u16string &wstr);
std::u16string utf8_to_utf16(std::string &str);
int write_utf16(std::ofstream &f, std::string &u8str);
int write_data(std::ofstream &f, std::vector<uint8_t> &data);

#endif // NEWTFMT_TOOLS_H

