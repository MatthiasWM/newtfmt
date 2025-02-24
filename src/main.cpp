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
//std::string input_pkg_name { "/Users/matt/Azureus/unna/games/SuperNewtris2.0/SNewtris.pkg" };
//std::string input_pkg_name { "/Users/matt/Azureus/unna/games/DeepGreen1.0b3/deepgreen10b3.pkg" }; // contains relocation data
//std::string input_pkg_name { "/Users/matt/Azureus/unna/games/GoldTeeAtBighorn/Goldtee.pkg" }; // Contains Floatin Point values
std::string input_pkg_name { "/Users/matt/Azureus/unna/games/NewTiles1.2/newtiles-1_2.pkg" }; // 'package1'

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
 Run our application with hardcoded file names for now.
 \param[in] argc, argv
 */
int main(int argc, const char * argv[])
{
  if (argc==2) {
    input_pkg_name = argv[1];
  }

  std::cout << "Testing package \"" << input_pkg_name << "\"." << std::endl;

  pkg::Package my_pkg;

  if (my_pkg.load(input_pkg_name) < 0) {
    std::cout << "ERROR reading package file." << std::endl;
    return 0;
  }
#if 0
  if (my_pkg.writeAsm("/Users/matt/dev/newtfmt.git/mines.s") < 0) {
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
  if (my_pkg.compareContents("/Users/matt/dev/newtfmt.git/mines.pkg") < 0) {
    std::cout << "ERROR comparing the original package and the new package contents." << std::endl;
    return 0;
  }
#endif
#if 0
  if (my_pkg.compareFile("/Users/matt/dev/newtfmt.git/mines.pkg") < 0) {
    std::cout << "ERROR comparing the binaries of the original package and the new package." << std::endl;
    return 0;
  }
#endif
  nos::Ref nos_pkg = my_pkg.toNOS();
  nos::Print(nos_pkg);
  std::cout << "OK." << std::endl;
  return 0;
}

#if 0

// Some Globals:
//  prettyPrint: true,
//  printDepth: 3,
//  printLength: nil,

// Using Clang on ARM64, these are all indeed compile time constants!
constexpr nos::Symbol gSymObjArray { "array" };
constexpr nos::Ref gSymArray { gSymObjArray };
constexpr nos::Ref r32 { 32 };

// Creating a read-only Array
constexpr nos::Ref aa[] = { 3, 4, 5 };
constexpr nos::Array a { gSymArray, 3, aa };

// Creating a read-only Frame
constexpr nos::Symbol gSymObjTop { "top" };
constexpr nos::Ref gSymTop { gSymObjTop };
constexpr nos::Symbol gSymObjLeft { "left" };
constexpr nos::Ref gSymLeft { gSymObjLeft };
constexpr nos::Ref f_map_symbols[] = { nos::RefNIL, gSymTop, gSymLeft };
constexpr nos::Array f_map { 0, 3, f_map_symbols };
constexpr nos::Object v205 { 20.5 };
constexpr nos::Ref f_values[] = { 10, v205 };
constexpr nos::Frame f { f_map, 2, f_values };

// Create a read-only array
constexpr nos::Array gArray { gSymArray, 2, f_values };
constexpr nos::Ref gRefArray { gArray };

constexpr nos::Object gObjHello { "Hello world!" };
constexpr nos::Ref gRefHello { gObjHello };

constexpr nos::Object gObjPi { 3.141592654 };
constexpr nos::Ref gRefPi { gObjPi };

int main(int argc, const char * argv[])
{
  (void)argc; (void)argv;

  nos::Print(gSymArray);  // compile time symbol
  nos::Print(a);          // test arrays
  nos::Print(f);          // test arrays
  nos::Print(r32);        // compile time integer
  nos::Print(U'ü');       // support for lower 8 bit unicode characters
  nos::Print(U'😀');      // support for full sized unicode characters
  nos::Print(gRefHello);  // static strings
  nos::Print(gRefPi);     // floating point values
  nos::Print(gArray);     // array
  nos::Print(gRefArray);  // array

  return 0;
}

#endif
