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


#include "package/package.h"
#include "package/package_bytes.h"
#include "package/part_data.h"
#include "package/part_entry.h"
#include "package/relocation_data.h"

#include "nos/ref.h"
#include "nos/objects.h"

#include "tools/tools.h"

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <locale>
#include <codecvt>


const std::string gnu_as { "/opt/homebrew/bin/arm-none-eabi-as" };
const std::string gnu_objcopy { "/opt/homebrew/bin/arm-none-eabi-objcopy" };

//std::string input_pkg_name { "/Users/matt/Azureus/unna/games/Mines/Mines.pkg" };
std::string input_pkg_name { "/Users/matt/Azureus/unna/games/SuperNewtris2.0/SNewtris.pkg" };

// TODO: don't use globals
std::string my_pkg_name;
pkg::PackageBytes my_pkg_bytes;
pkg::Package my_pkg;

/**
 Read a Package file and create the internal data representation.
 \param[in] package_file_name path and name
 \return 0 if successful
 */
int readPackage(std::string package_file_name)
{
  my_pkg_name = package_file_name;
  std::ifstream source_file { package_file_name, std::ios::binary };
  if (source_file) {
    my_pkg_bytes.assign(std::istreambuf_iterator<char>{source_file}, {});
    std::cout << "readPackage: \"" << package_file_name << "\" package read (" << my_pkg_bytes.size() << " bytes)." << std::endl;
    return my_pkg.load(my_pkg_bytes);
  }
  std::cout << "readPackage: Unable to read file \"" << package_file_name << "\"." << std::endl;
  return -1;
}

/**
 Write a Package as an ARM32 assembler file.
 \param[in] assembler_file_name path and name
 \return 0 if successful
 */
int writeAsm(std::string assembler_file_name)
{
  std::ofstream asm_file { assembler_file_name };
  if (asm_file.fail()) {
    std::cout << "writeAsm: Unable to write assembler file \"" << assembler_file_name << "\"." << std::endl;
    return -1;
  }

  asm_file << "@" << std::endl;
  asm_file << "@ Assembler file generated from Newton Package" << std::endl;
  asm_file << "@" << std::endl << std::endl;

  asm_file << "\t.macro\tref_magic index\n"
           << "\t.int\t((\\index)<<2)|3\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_integer value\n"
           << "\t.int\t((\\value)<<2)\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_pointer label\n"
           << "\t.int\t\\label + 1\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_pointer_invalid offset\n"
           << "\t.int\t\\offset\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_unichar value\n"
           << "\t.int\t((\\value)<<4)|10\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_nil\n"
           << "\t.int\t0x00000002\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro\tref_true\n"
           << "\t.int\t0x0000001a\n"
           << "\t.endm\n\n";


  asm_file << "\t.file\t\"" << my_pkg_name << "\"" << std::endl;
  asm_file << "\t.data" << std::endl << std::endl;


  int skip = my_pkg.writeAsm(asm_file);
  if (skip < (int)my_pkg_bytes.size()) {
    std::cout << "WARNING: Package has " << my_pkg_bytes.size()-skip << " more bytes than defined." << std::endl;
    asm_file << "@ ===== Extra data in file" << std::endl;
    for (auto it = my_pkg_bytes.begin()+skip; it != my_pkg_bytes.end(); ++it) {
      uint8_t b = *it;
      asm_file << "\t.byte\t0x"
      << std::setw(2) << std::setfill('0') << std::hex << (int)b
      << "\t@ " << (char)( ((b > 32) && (b < 127)) ? b : '.' )
      << std::endl;
    }
  }

//  std::cout << "writeAsm: Wrote \"" << assembler_file_name << "\"." << std::endl;
  return 0;
}

/**
 Call the GNU assembler to create an object file form the assembler file.
 \param[in] assembler_file_name
 \param[in] object_file_name
 \return 0 if successful
 */
int asmToObj(std::string assembler_file_name, std::string object_file_name)
{
  std::string cmd = gnu_as + " -march=armv4 -mbig-endian "
                           + "\"" + assembler_file_name + "\" "
                           + "-o \"" + object_file_name + "\"";
  if (std::system(cmd.c_str()) != 0) {
    std::cout << "asmToObj: Unable to generate object file:" << std::endl
              << "  " << cmd << std::endl;
    return -1;
  }
  
//  std::cout << "asmToObj: Wrote \"" << object_file_name << "\"." << std::endl;
  return 0;
}

/**
 Call GNU objcopy to extract the binary .data segment, creating a new Package.
 \param[in] object_file_name
 \param[in] new_package_name file path and name
 \return 0 if successful
 */
int objToBin(std::string object_file_name, std::string new_package_name)
{
  std::string cmd = gnu_objcopy + " -O binary -j .data "
                                + "\"" + object_file_name + "\" "
                                + "\"" + new_package_name + "\"";
  if (std::system(cmd.c_str()) != 0) {
    std::cout << "objToBin: Unable to generate binary file:" << std::endl
    << "  " << cmd << std::endl;
    return -1;
  }

//  std::cout << "objToBin: Wrote \"" << new_package_name << "\"." << std::endl;
  return 0;
}

/**
 Compare a package file with the currently loaded package byte by byte.
 \param[in] new_package_name path and file name
 \return 0 if data is indentical
 */
int compareBinaries(std::string new_package_name)
{
  std::vector<uint8_t> new_pkg;
  std::ifstream new_file { new_package_name, std::ios::binary };
  if (new_file) {
    new_pkg.assign(std::istreambuf_iterator<char>{new_file}, {});
    if (new_pkg == my_pkg_bytes) {
//      std::cout << "compareBinaries: Packages are identical." << std::endl;
      std::cout << "OK." << std::endl;
    } else {
      int i, n = std::min((int)new_pkg.size(), (int)my_pkg_bytes.size());
      for (i=0; i<n; ++i) {
        if (new_pkg[i] != my_pkg_bytes[i]) break;
      }
      std::cout << "ERROR: compareBinaries: Packages differ starting at 0x"
      << std::setw(8) << std::setfill('0') << std::hex << i << std::dec
      << " = " << i << "!" << std::endl;
    }
    std::cout << std::endl;
    return 0;
  }
  std::cout << "compareBinaries: Unable to read new file \"" << new_package_name << "\"." << std::endl;
  return -1;
}

/**
 Run our application with hardcoded file names for now.
 \param[in] argc, argv
 */
int main_package_test(int argc, const char * argv[])
{
  if (argc==2) {
    input_pkg_name = argv[1];
  }
  if (readPackage(input_pkg_name) < 0) {
    std::cout << "ERROR reading package file." << std::endl;
    return 0;
  }
  if (writeAsm("/Users/matt/dev/newtfmt.git/mines.s") < 0) {
    std::cout << "ERROR writing assembler file." << std::endl;
    return 0;
  }
  if (asmToObj("/Users/matt/dev/newtfmt.git/mines.s", "/Users/matt/dev/newtfmt.git/mines.o") < 0) {
    std::cout << "ERROR calling assembler and creating object file." << std::endl;
    return 0;
  }
  if (objToBin("/Users/matt/dev/newtfmt.git/mines.o", "/Users/matt/dev/newtfmt.git/mines.pkg") < 0) {
    std::cout << "ERROR extracting binary data from object file." << std::endl;
    return 0;
  }
  if (compareBinaries("/Users/matt/dev/newtfmt.git/mines.pkg") < 0) {
    std::cout << "ERROR comparing the original package and the new package." << std::endl;
    return 0;
  }
  return 0;
}

// Using Clang on ARM64, these are all indeed compile time constants!
constexpr nos::Object gSymObjArray { 0x2222, "array" }; // TODO: fix the hash!
constexpr nos::Ref gSymArray { gSymObjArray };
constexpr nos::Ref r32 { 32 };

// Creating a read-only Array
constexpr nos::Ref aa[] = { 3, 4, 5 };
constexpr nos::Object a { nos::Object::Array(gSymArray, 3, aa) };

// Creating a read-only Frame
constexpr nos::Object gSymObjTop { 0x2222, "top" }; // TODO: fix the hash!
constexpr nos::Ref gSymTop { gSymObjTop };
constexpr nos::Object gSymObjLeft { 0x2222, "left" }; // TODO: fix the hash!
constexpr nos::Ref gSymLeft { gSymObjLeft };
constexpr nos::Ref f_map_symbols[] = { nos::RefNIL, gSymTop, gSymLeft };
constexpr nos::Object f_map { nos::Object::Array(0, 3, f_map_symbols) };
constexpr nos::Ref f_values[] = { 10, 20 };
constexpr nos::Object f { nos::Object::Frame(f_map, 2, f_values) };


int main(int argc, const char * argv[])
{
  (void)argc; (void)argv;

  nos::Print(gSymArray);  // compile time symbol
  nos::Print(a);          // test arrays
  nos::Print(f);          // test arrays
  nos::Print(r32);        // compile time integer
  nos::Print(U'ü');       // support for lower 8 bit unicode characters
  nos::Print(U'😀');      // support for full sized unicode characters

  return 0;
}
