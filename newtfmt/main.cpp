//
//  main.cpp
//  newtfmt
//
//  Created by Matthias Melcher on 24.01.25.
//


#include "package.h"
#include "package_bytes.h"
#include "part_data.h"
#include "part_entry.h"
#include "relocation_data.h"
#include "tools.h"

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <locale>
#include <codecvt>


const std::string gnu_as { "/opt/homebrew/bin/arm-none-eabi-as" };
const std::string gnu_objcopy { "/opt/homebrew/bin/arm-none-eabi-objcopy" };

std::string pkg_name;

ns::PackageBytes pkg_bytes;
ns::Package pkg;

/**
 Read a Package file and create the internal data representation.
 \param[in] package_file_name path and name
 \return 0 if successful
 */
int readPackage(std::string package_file_name)
{
  pkg_name = package_file_name;
  if (std::ifstream source_file { package_file_name, std::ios::binary }; source_file) {
    pkg_bytes.assign(std::istreambuf_iterator<char>{source_file}, {});
    std::cout << "readPackage: package read (" << pkg_bytes.size() << " bytes)." << std::endl;
    pkg.load(pkg_bytes);
    return 0;
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

  asm_file << "\t.macro  ref_magic index\n"
           << "\t.int\t((\\index)<<2)|3\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro  ref_integer value\n"
           << "\t.int\t((\\value)<<2)\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro  ref_pointer label\n"
           << "\t.int\t\\label + 1\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro  ref_unichar value\n"
           << "\t.int\t((\\value)<<4)|10\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro  ref_nil\n"
           << "\t.int\t0x00000002\n"
           << "\t.endm\n\n";

  asm_file << "\t.macro  ref_true\n"
           << "\t.int\t0x0000001a\n"
           << "\t.endm\n\n";


  asm_file << "\t.file\t\"" << pkg_name << "\"" << std::endl;
  asm_file << "\t.data" << std::endl << std::endl;

  int skip = pkg.writeAsm(asm_file);
  for (auto it = pkg_bytes.begin()+skip; it != pkg_bytes.end(); ++it) {
    uint8_t b = *it;
    asm_file << "\t.byte\t0x"
             << std::setw(2) << std::setfill('0') << std::hex << (int)b
             << "\t@ " << (char)( ((b > 32) && (b < 127)) ? b : '.' )
             << std::endl;
  }

  std::cout << "writeAsm: Wrote \"" << assembler_file_name << "\"." << std::endl;
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
  
  std::cout << "asmToObj: Wrote \"" << object_file_name << "\"." << std::endl;
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

  std::cout << "objToBin: Wrote \"" << new_package_name << "\"." << std::endl;
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
    if (new_pkg == pkg_bytes)
      std::cout << "compareBinaries: Packages are identical." << std::endl;
    else
      std::cout << "compareBinaries: WARNING! Packages are different!" << std::endl;
    return 0;
  }
  std::cout << "compareBinaries: Unable to read new file \"" << new_package_name << "\"." << std::endl;
  return -1;
}

/**
 Run our application with hardcoded file names for now.
 \param[in] argc, argv
 */
int main(int argc, const char * argv[])
{
  if (readPackage("/Users/matt/Azureus/unna/games/Mines/Mines.pkg") < 0) {
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
