//
//  main.cpp
//  newtfmt
//
//  Created by Matthias Melcher on 24.01.25.
//

/*

 The following code creates a raw binary (no ELF or PE.. just raw):

 gcc -nostartfiles -c -o hello.o hello.c
 ld -o hello1 hello.o --oformat binary

 You can try the following code out with it:

 void _start()
 {
 while(1);
 }

 The resulting raw binary is 5 bytes.

 It is 2.1M here (on x86_64), and 4.3k with -m32 when using gcc 4.4.2.
 Apparently it includes the dynamic linker inside it.
 Now how do you do this in clang/llvm?
 Apparently llvm-ld doesn't have a "--oformat binary" option.
 Anybody know how to do this with llvm?

 Simple, use ld on the .o files, as you would with gcc.
 An example with clang:
 $ clang -nostartfiles -c x.c -o x.o
 $ ld -o x x.o --oformat binary

 Or to get a 32-bit one:
 $ /llvm-git/obj/Release/bin/clang -nostartfiles -c x.c -o x.o -m32
 $ ld -o x1 x.o --oformat binary -melf_i386

 This one is indeed 5 bytes long.


 fl_system(cmd);
 -nostdlib -nostartfiles -nodefaultlibs -static -c

 run `arm-none-eabi-as -march=armv4 -mbig-endian test.s -o test.o`
 run `arm-none-eabi-objdump -d test.o >test.dis`
  "\"%s\" -d --show-raw-insn \"%s\" >\"%s\" 2>\"%s\"",
  gApp->GetSettings()->mDevObjDumpPath,
  objfilename, disfilename, errfilename);

 NewtMakeBinaryFromARMFile()
 run `arm-none-eabi-as -march=armv4 -mbig-endian test.s -o test.o`
 run `arm-none-eabi-objcopy -O binary -j .text test.o test`


 https://sourceware.org/binutils/docs/as/Pseudo-Ops.html
      .text
      .data
      .align 4, fill,
 labelname:
      .quad 1 ; 64 bits
      .long 787 ; == .int, 32 bits
      .byte
      .byte    0x00,0x42,0x22
      .short  'I','n','t','e','r','n','a','l',0 ; = .word
      .ascii "TExt\0"
      .asciz "Text"
      .macro name
      args in $($0) etc. ??
      .endmacro
      .zero n
      .warning "string" ; .error .print
      .string16 "Hello"
      .base64 "string" ; output long binary data
      .dc.d 3.1415 ; emit a double size floating point value
      .double 3.1415
      .file filename
      .incbin "file"
      .include "file"
 */

#include <iostream>
#include <fstream>
#include <ios>
#include <cstdlib>
#include <locale>
#include <codecvt>


const std::string gnu_as { "/opt/homebrew/bin/arm-none-eabi-as" };
const std::string gnu_objcopy { "/opt/homebrew/bin/arm-none-eabi-objcopy" };
std::string pkg_name;


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

namespace ns {

class PackageBytes : public std::vector<uint8_t> {
  PackageBytes::iterator it_;
public:
  PackageBytes() = default;
  void rewind() { it_ = begin(); }
  int tell() { return (int)(it_ - begin()); }
  bool eof() { return it_ == end(); }
  uint8_t get_ubyte() { return *it_++; }
  uint16_t get_ushort() { uint16_t v; v = ((*it_++)<<8)|(*it_++); return v; }
  uint32_t get_uint() { uint32_t v; v = ((*it_++)<<24)|((*it_++)<<16)|((*it_++)<<8)|(*it_++); return v; }
  std::string get_cstring(int n, bool trailing_nul=true) {
    PackageBytes::iterator it_start = it_;
    if (trailing_nul) {
      it_ += n+1;
      return std::string(it_start, it_-1);
    } else {
      it_ += n;
      return std::string(it_start, it_);
    }
  }
  std::string get_ustring(int n, bool trailing_nul=true) {
    std::u16string s;
    for (int i=0; i<n; i++) s += get_ushort();
    if (trailing_nul) get_ushort();
    return utf16_to_utf8(s);
  }
  std::vector<uint8_t> get_data(int n) {
    auto it_start = it_;
    it_ += n;
    return std::vector<uint8_t>(it_start, it_);
  }
  void align(int n) { }
};

class RelocationData {
  //      repeat num_entries_
  //      .balign 4
  //      struct RelocationSet {
  //        UShort pageNumber;
  //        UShort offsetCount;
  //        /* Byte offsets[]; */
  //      };
  uint32_t reserved_ {0};
  uint32_t size_ {0};
  uint32_t page_size_ {0};
  uint32_t num_entries_ {0};
  uint32_t base_address_ {0};
  std::vector<uint8_t> data_;
public:
  RelocationData() = default;
  int load(PackageBytes &p) {
    reserved_ = p.get_uint();
    size_ = p.get_uint();
    page_size_ = p.get_uint();
    num_entries_ = p.get_uint();
    base_address_ = p.get_uint();
    data_ = p.get_data(size_-20);
    return 0;
  }
  int writeAsm(std::ofstream &f) {
    f << "@ ===== Relocation Data" << std::endl;
    f << "\t.int\t" << reserved_ << "\t@ reserved" << std::endl;
    f << "\t.int\t" << size_ << "\t@ size" << std::endl;
    f << "\t.int\t" << page_size_ << "\t@ page_size" << std::endl;
    f << "\t.int\t" << num_entries_ << "\t@ num_entries" << std::endl;
    f << "\t.int\t" << base_address_ << "\t@ base_address" << std::endl;
    write_data(f, data_);
    f << std::endl;
    return size_;
  }
};

class PartEntry;

class PartData {
protected:
  PartEntry &part_entry_;
public:
  PartData(PartEntry &part_entry) : part_entry_(part_entry) { }
  virtual ~PartData() = default;
  virtual int load(PackageBytes &p) = 0;
  virtual int writeAsm(std::ofstream &f) = 0;
};

class PartDataGeneric : public PartData {
  std::vector<uint8_t> data_;
public:
  PartDataGeneric(PartEntry &part_entry) : PartData(part_entry) { }
  ~PartDataGeneric() override = default;
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f) override;
};

class PartEntry {
  int index_;
  uint32_t offset_ {0};
  uint32_t size_ {0};
  uint32_t size2_ {0};
  std::string type_;
  uint32_t reserved_ {0};
  uint32_t flags_ {0};
  uint16_t info_offset_ {0};
  uint16_t info_length_ {0};
  uint16_t compressor_offset_ {0};
  uint16_t compressor_length_ {0};
  std::string info_;
  std::shared_ptr<PartData> part_data_;
public:
  PartEntry(int ix) : index_(ix) { }
  int size() { return size_; }
  int index() { return index_; }
  int load(PackageBytes &p) {
    offset_ = p.get_uint();
    size_ = p.get_uint();
    size2_ = p.get_uint();
    if (size_ != size2_)
      std::cout << "WARNING: Part Entry " << index_ << ": Size and size2 differ.\n";
    type_ = p.get_cstring(4, false);
    // 'form' 'book' 'dict' 'auto' 'comm'
    if (type_=="book" || type_=="dict" || type_=="comm")
      std::cout << "WARNING: Part Entry " << index_ << ": unsupported type \"" << type_ << "\"\n";
    if (type_!="form" && type_!="book" && type_!="dict" && type_!="comm")
      std::cout << "WARNING: Part Entry " << index_ << ": unknown type \"" << type_ << "\"\n";
    reserved_ = p.get_uint();
    flags_ = p.get_uint();
    if ((flags_ & 3) == 3)
      std::cout << "WARNING: Part Entry " << index_ << ": unknown type in flags: "
      << (flags_ & 3) << std::endl;
    if (flags_ & 0xfffffe0c)
      std::cout << "WARNING: Part Entry " << index_ << ": unknown flag: "
        << std::setw(8) << std::setfill('0') << std::hex
        << (flags_ & 0xfffffe0c) << std::dec << std::endl;
    info_offset_ = p.get_ushort();
    info_length_ = p.get_ushort();
    compressor_offset_ = p.get_ushort();
    compressor_length_ = p.get_ushort();

    if ((flags_ & 3) == 1) // kNOSPart
      part_data_ = std::make_shared<PartDataGeneric>(*this); // TODO: NOS Part Data
    else
      part_data_ = std::make_shared<PartDataGeneric>(*this);

    return 0;
  }
  int loadInfo(PackageBytes &p) {
    if (info_length_ > 0)
      info_ = p.get_cstring(info_length_, false);
    return 0;
  }
  int loadPartData(PackageBytes &p) {
    return part_data_->load(p);
  }
  int writeAsm(std::ofstream &f) {
    f << "@ ===== Part Entry " << index_ << std::endl;
    f << "\t.int\t" << offset_ << "\t@ offset" << std::endl;
    f << "\t.int\t" << size_ << "\t@ size" << std::endl;
    f << "\t.int\t" << size2_ << "\t@ size2" << std::endl;
    f << "\t.ascii\t\"" << type_ << "\"\t@ type\n";
    f << "\t.int\t" << reserved_ << "\t@ reserved" << std::endl;
    f << "\t.int\t0x" << std::setw(8) << std::setfill('0') << std::hex << flags_ << std::dec << "\t@ flags\n";
    static const std::string lut[] = { "kProtocolPart", "kNOSPart", "kRawPart", "UNKNOWN"};
    f << "\t\t@ " << lut[flags_ & 3] << std::endl;
    if (flags_ & 0x000001f0) {
      f << "\t\t@";
      if (flags_ & 0x00000010) f << " kAutoLoadPartFlag";
      if (flags_ & 0x00000020) f << " kAutoRemovePartFlag";
      if (flags_ & 0x00000040) f << " kCompressedFlag";
      if (flags_ & 0x00000080) f << " kNotifyFlag";
      if (flags_ & 0x00000100) f << " kAutoCopyFlag";
      f << std::endl;
    }
    if (flags_ & 0xfffffe0c)
      f << "\t@ WARNING unknown flag: "
        << std::setw(8) << std::setfill('0') << std::hex
        << (flags_ & 0xfffffe0c) << std::dec << std::endl;
    f << "\t.short\t" << info_offset_ << ", " << info_length_ << "\t@ info" << std::endl;
    f << "\t.short\t" << compressor_offset_ << ", " << compressor_length_ << "\t@ compressor" << std::endl;
    f << std::endl;
    return 32;
  }
  int writeAsmInfo(std::ofstream &f) {
    f << "@ ----- Part " << index_ << " Info" << std::endl;
    f << "part" << index_ << "info_start:" << std::endl;
    if (info_length_)
      f << "\t.ascii\t\"" << info_ << "\"\t@ info" << std::endl;
    f << "part" << index_ << "info_end:" << std::endl << std::endl;
    return info_length_;
  }
  int writeAsmPartData(std::ofstream &f) {
    return part_data_->writeAsm(f);
  }
};

int PartDataGeneric::load(PackageBytes &p) {
  data_ = p.get_data(part_entry_.size());
  return 0;
}

int PartDataGeneric::writeAsm(std::ofstream &f) {
  f << "@ ===== Part " << part_entry_.index() << " Data Generic" << std::endl;
  write_data(f, data_);
  f << "\t.balign\t4" << std::endl << std::endl;
  return part_entry_.size();
}


class Package {
  std::string signature_;
  std::string type_;
  uint32_t flags_ {0};
  uint32_t version_ {0};
  uint16_t copyright_start_ {0};
  uint16_t copyright_length_ {0};
  uint16_t name_start_ {0};
  uint16_t name_length_ {0};
  uint32_t size_ {0};
  uint32_t date_ {0};
  uint32_t reserved2_ {0};
  uint32_t reserved3_ {0};
  uint32_t directory_size_ {0};
  uint32_t num_parts_ {0};
  uint32_t vdata_start_ {0};
  uint32_t info_start_ {0};
  uint32_t info_length_ {0};
  std::vector<PartEntry> part_;
  std::string copyright_;
  std::string name_;
  std::vector<uint8_t> info_;
  RelocationData relocation_data_;
public:
  Package() = default;
  int load(PackageBytes &p) {
    p.rewind();
    signature_ = p.get_cstring(8, false);
    if ((signature_ != "package0") && (signature_ != "package1"))
      std::cout << "WARNING: unknown signature \"" << signature_ << "\"\n";
    type_ = p.get_cstring(4, false);
    flags_ = p.get_uint();
    if (flags_ & 0x09ffffff)
      std::cout << "WARNING: unknown flag: "
      << std::setw(4) << std::setfill('0') << std::hex
      << (flags_ & 0x09ffffff) << std::endl;
    version_ = p.get_uint();
    copyright_start_ = p.get_ushort();
    if (copyright_start_ != 0)
      std::cout << "WARNING: Copyright offset should be 0.\n";
    copyright_length_ = p.get_ushort();
    name_start_ = p.get_ushort();
    if (name_start_ != copyright_start_ + copyright_length_)
      std::cout << "WARNING: Name offset should be " << copyright_start_ + copyright_length_
                << ", but it's " << name_start_ << ".\n";
    name_length_ = p.get_ushort();
    if (name_length_ == 0)
      std::cout << "WARNING: Name length can't be 0.\n";
    size_ = p.get_uint();
    if (size_ != p.size())
      std::cout << "WARNING: size entry does not match file size (" << size_ << "!=" << p.size() << ").\n";
    date_ = p.get_uint();
    reserved2_ = p.get_uint();
    if (reserved2_ != 0)
      std::cout << "WARNING: Reserved2 should be 0.\n";
    reserved3_ = p.get_uint();
    if (reserved3_ != 0)
      std::cout << "WARNING: Reserved3 should be 0.\n";
    directory_size_ = p.get_uint();
    num_parts_ = p.get_uint();
    if (num_parts_ > 32)
      std::cout << "WARNING: Unlikely number of parts (" << num_parts_ << ").\n";
    for (int i = 0; i < num_parts_; ++i) {
      part_.push_back(PartEntry(i));
      part_[i].load(p);
    }
    vdata_start_ = p.tell();
    if (copyright_length_) {
      copyright_ = p.get_ustring(copyright_length_/2-1);
    }
    if (name_length_) {
      name_ = p.get_ustring(name_length_/2-1);
    }
    for (auto &part: part_) part.loadInfo(p);
    // NTK sneaks a message into the variable data area after the last info
    // and before the relocation data and parts start.
    // "Newtontm™ ToolKit Package © 1992-1997, Apple Computer, Inc."
    info_length_ = directory_size_ - p.tell(); // 58 bytes + 2 bytes padding
    info_ = p.get_data(info_length_);

    // Relocation Data if kRelocationFlag is set
    if (flags_ & 0x04000000) {
      relocation_data_.load(p);
    }

    // Part Data
    for (auto &part: part_) part.loadPartData(p);
//    NOS Part:
//    00001041 Array
//    00000001 Alignment is 4 bytes (0 is 8 bytes)
//    00000002 NIL
//    000000e9 starting frame pointer (offset into part +1)

    return 0;
  }
  int writeAsm(std::ofstream &f) {
    f << "@ ===== Package Header" << std::endl;
    f << "\t.ascii\t\"" << signature_ << "\"\t@ signature\n";
    f << "\t.ascii\t\"" << type_ << "\"\t@ type\n";
    f << "\t.int\t0x" << std::setw(8) << std::setfill('0') << std::hex << flags_ << std::dec << "\t@ flags\n";
    if (flags_ & 0xf0000000) {
      f << "\t\t@";
      if (flags_ & 0x80000000) f << " kAutoRemoveFlag";
      if (flags_ & 0x40000000) f << " kCopyProtectFlag";
      if (flags_ & 0x20000000) f << " kInvisibleFlag";
      if (flags_ & 0x10000000) f << " kNoCompressionFlag";
      f << std::endl;
    }
    if (flags_ & 0x06000000) {
      f << "\t\t@";
      if (flags_ & 0x04000000) f << " kRelocationFlag";
      if (flags_ & 0x02000000) f << " kUseFasterCompressionFlag";
      f << std::endl;
    }
    if (flags_ & 0x09ffffff) 
      f << "\t@ WARNING unknown flag: "
        << std::setw(4) << std::setfill('0') << std::hex
        << (flags_ & 0x09ffffff) << std::dec << std::endl;
    f << "\t.int\t" << version_ << "\t@ version\n";
//  f << "\t.short\t" << copyright_start_ << ", " << copyright_length_ << "\t@ copyright\n";
    f << "\t.short\tpkg_copyright_start-pkg_data, pkg_copyright_end-pkg_copyright_start\t@ copyright\n";
//  f << "\t.short\t" << name_start_ << ", " << name_length_ << "\t@ name\n";
    f << "\t.short\tpkg_name_start-pkg_data, pkg_name_end-pkg_name_start\t@ name\n";
    f << "\t.int\t" << size_ << "\t@ size\n";
    f << "\t.int\t0x" << std::setw(8) << std::hex << date_ << std::dec << "\t@ date\n";
    f << "\t.int\t0x" << std::setw(8) << std::hex << reserved2_ << std::dec << "\t@ reserverd2\n";
    f << "\t.int\t0x" << std::setw(8) << std::hex << reserved3_ << std::dec << "\t@ reserverd3\n";
    f << "\t.int\t" << directory_size_ << "\t@ directory_size\n";
    f << "\t.int\t" << num_parts_ << "\t@ num_parts\n";
    f << std::endl;
    int bytes = 52;
    for (int i = 0; i < num_parts_; ++i) {
      bytes += part_[i].writeAsm(f);
    }
    f << "@ ===== Copyright" << std::endl;
    f << "pkg_data:" << std::endl << std::endl;

    f << "@ ----- Copyright" << std::endl;
    f << "pkg_copyright_start:" << std::endl;
    if (copyright_length_)
      bytes += write_utf16(f, copyright_);
    f << "pkg_copyright_end:" << std::endl << std::endl;

    f << "@ ----- Name" << std::endl;
    f << "pkg_name_start:" << std::endl;
    if (name_length_)
      bytes += write_utf16(f, name_);
    f << "pkg_name_end:" << std::endl << std::endl;

    for (auto &part: part_) bytes += part.writeAsmInfo(f);

    if (info_.size() > 0) {
      f << "@ ----- Package Info" << std::endl;
      bytes += write_data(f, info_);
      f << std::endl;
    }

    f << "\t.balign\t4, 0xff" << std::endl << std::endl;

    // Relocation Data if kRelocationFlag is set
    if (flags_ & 0x04000000) {
      relocation_data_.writeAsm(f);
    }

    for (auto &part: part_) bytes += part.writeAsmPartData(f);

    return bytes;
  }
};

} // namespace ns

ns::PackageBytes pkg_bytes;
ns::Package pkg;

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

int compareBinaries(std::string new_package_name)
{
  std::vector<uint8_t> new_pkg;
  if (std::ifstream new_file { new_package_name, std::ios::binary }; new_file) {
    new_pkg.assign(std::istreambuf_iterator<char>{new_file}, {});
    if (new_pkg == pkg_bytes)
      std::cout << "compareBinaries: Packages are equal." << std::endl;
    else
      std::cout << "compareBinaries: WARNING! Packages are different!" << std::endl;
    return 0;
  }
  std::cout << "compareBinaries: Unable to read new file \"" << new_package_name << "\"." << std::endl;
  return -1;
}

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
