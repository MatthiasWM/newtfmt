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


#ifndef NEWTFMT_TOOLS_TOOLS_H
#define NEWTFMT_TOOLS_TOOLS_H

#include <string>

std::string utf16_to_utf8(std::u16string &wstr);
std::u16string utf8_to_utf16(std::string &str);
int write_utf16(std::ofstream &f, std::string &u8str);
int write_data(std::ofstream &f, std::vector<uint8_t> &data);

#endif // NEWTFMT_TOOLS_TOOLS_H

