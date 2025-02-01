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


#ifndef NEWTFMT_PACKAGE_PART_DATA_H
#define NEWTFMT_PACKAGE_PART_DATA_H

#include <ios>
#include <cstdlib>
#include <vector>
#include <map>

namespace pkg {

class PartEntry;
class PackageBytes;

class PartData {
protected:
  PartEntry &part_entry_;
public:
  PartData(PartEntry &part_entry) : part_entry_(part_entry) { }
  virtual ~PartData() = default;
  virtual int load(PackageBytes &p) = 0;
  virtual int writeAsm(std::ofstream &f) = 0;
  int index();
};

class PartDataGeneric : public PartData {
  std::vector<uint8_t> data_;
public:
  PartDataGeneric(PartEntry &part_entry) : PartData(part_entry) { }
  ~PartDataGeneric() override = default;
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f) override;
};

class PartDataNOS;

class Object {
protected:
  std::string label_;
  uint32_t offset_{ 0 };
  uint32_t type_ {0};
  uint32_t flags_ {0};
  uint32_t size_ {0};
  uint32_t ref_cnt_ {0};
  uint32_t class_{ 0 };
public: // TODO: hack
  std::vector<uint8_t> padding_;
public:
  static std::shared_ptr<Object> peek(PackageBytes &p, uint32_t offset);
  Object(uint32_t offset) : offset_(offset) { }
  virtual ~Object() = default;
  virtual int load(PackageBytes &p);
  void loadPadding(PackageBytes &p, uint32_t start, uint32_t align);
  virtual int writeAsm(std::ofstream &f, PartDataNOS &p);
  virtual void makeAsmLabel(PartDataNOS &p);
  std::string &label() { return label_; }
  uint32_t type() const { return type_; }
  uint32_t offset() const { return offset_; }
  uint32_t size() const { return size_; }
};

class ObjectBinary : public Object {
  std::vector<uint8_t> data_;
public:
  ObjectBinary(uint32_t offset) : Object(offset) { }
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f, PartDataNOS &p) override;
};

class ObjectSymbol : public Object {
  uint32_t hash_{ 0 };
  std::string symbol_;
public:
  ObjectSymbol(uint32_t offset) : Object(offset) { }
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f, PartDataNOS &p) override;
  void makeAsmLabel(PartDataNOS &p) override;
};

class ObjectSlotted : public Object {
protected:
  std::vector<uint32_t> ref_list_;
public:
  ObjectSlotted(uint32_t offset) : Object(offset) { }
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f, PartDataNOS &p) override;
};

class ObjectMap : public ObjectSlotted {
public:
  ObjectMap(uint32_t offset) : ObjectSlotted(offset) { }
  int writeAsm(std::ofstream &f, PartDataNOS &p) override;
};

class PartDataNOS : public PartData {
  std::map<uint32_t, std::shared_ptr<Object>> object_list_;
  std::map<std::string, ObjectSymbol*> label_list_;
  uint32_t align_{ 8 };
  uint32_t align_fill_{ 0xadbadbad };
public:
  PartDataNOS(PartEntry &part_entry) : PartData(part_entry) { }
  ~PartDataNOS() override = default;
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f) override;
  std::string asmRef(uint32_t ref);
  bool addLabel(std::string label, ObjectSymbol *symbol);
};

} // namespace pkg

#endif // NEWTFMT_PACKAGE_PART_DATA_H
