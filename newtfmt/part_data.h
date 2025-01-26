

#ifndef NEWTFMT_PART_DATA_H
#define NEWTFMT_PART_DATA_H

#include <ios>
#include <cstdlib>
#include <vector>
#include <map>

namespace ns {

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
  uint32_t offset_{ 0 };
  uint32_t type_ {0};
  uint32_t flags_ {0};
  uint32_t size_ {0};
  uint32_t ref_cnt_ {0};
  uint32_t class_{ 0 };
public:
  static std::shared_ptr<Object> peek(PackageBytes &p, uint32_t offset);
  Object(uint32_t offset) : offset_(offset) { }
  virtual ~Object() = default;
  virtual int load(PackageBytes &p);
  virtual int writeAsm(std::ofstream &f, PartDataNOS &p);
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
};

class ObjectSlotted : public Object {
  std::vector<uint32_t> ref_list_;
public:
  ObjectSlotted(uint32_t offset) : Object(offset) { }
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f, PartDataNOS &p) override;
};

class PartDataNOS : public PartData {
  std::map<uint32_t, std::shared_ptr<Object>> object_list_;
  uint32_t align_ {8};
public:
  PartDataNOS(PartEntry &part_entry) : PartData(part_entry) { }
  ~PartDataNOS() override = default;
  int load(PackageBytes &p) override;
  int writeAsm(std::ofstream &f) override;
  std::string asmRef(uint32_t ref);
};

} // namespace ns

#endif // NEWTFMT_PART_DATA_H
