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


#include "nos/objects.h"

#include <cassert>

using namespace nos;

nos::Object::Object(const std::string &str)
: t { Tag::binary, 0x10 }, size_{ (uint32_t)::strlen(str.c_str()) }, binary{ gSymString, ::strdup(str.c_str()) }
{ }

Index nos::SlottedObject::Length() const {
  if ((t.tag_==Tag::array) || (t.tag_==Tag::frame))
    return size()/sizeof(Ref);
  else
    return -1;
}

void nos::SlottedObject::SetLength(Index new_length)
{
  Index old_length = Length();
  Index avail = old_length + array.reserve_;

  if (new_length <= avail) {
    // TODO: we may want to shrink here if the difference is too much
    array.reserve_ = (uint32_t)(avail - new_length);
  } else {
    array.reserve_ = (new_length>16) ? 9 : 5; // Rather random values
    avail = new_length + array.reserve_;
    array.slot_ = (Ref*)::realloc(array.slot_, avail * sizeof(Ref));
  }
  size_ = (uint32_t)(new_length * sizeof(Ref));
  if (new_length > old_length) {
    for (Index i=old_length; i<new_length; i++) {
      assert((i >= 0) && (i < (Index)(size_/sizeof(Ref))));
      array.slot_[i] = RefNIL;
    }
  }
}

Ref nos::SlottedObject::GetSlot(Index i) const {
  if ((t.tag_==Tag::array) || (t.tag_==Tag::frame)) {
    if (i<(Index)(size()/sizeof(Ref))) {
      assert((i >= 0) && (i < (Index)(size_/sizeof(Ref))));
      return frame.slot_[i];
    } else {
      return RefNIL;
    }
  } else {
    return RefNIL;
  }
}

int nos::symcmp(const char *s1, const char *s2)
{
  for (;;) {
    unsigned char c1 = static_cast<unsigned char>(*s1++);
    unsigned char c2 = static_cast<unsigned char>(*s2++);
    if (c1 == 0) {
      if (c2 == 0)
        return 0;
      else
        return -1;
    }
    if (c2 == 0)
      return 1;
    c1 = std::tolower(c1);
    c2 = std::tolower(c2);
    if (c1 != c2) {
      if (c1 > c2)
        return 1;
      else
        return -1;
    }
  }
}

int nos::Object::SymbolCompare(const Object *other) const
{
  if (symbol.hash_ != other->symbol.hash_) {
    if (symbol.hash_ > other->symbol.hash_)
      return 1;
    else
      return -1;
  }
  return symcmp(symbol.string_, other->symbol.string_);
}

int nos::SymbolCompare(Ref sym1, Ref sym2)
{
  if (sym1 == sym2)
    return 0;
  Object *obj1 = sym1.GetObject();
  Object *obj2 = sym2.GetObject();
  return obj1->SymbolCompare(obj2);
}

int nos::Object::Print(PrintState &ps) const
{
  switch (t.tag_) {
    case Tag::binary:
      // TODO: binary.class_ is not necessarily an object!
      if (binary.class_.GetObject()->SymbolCompare(&gSymObjString)==0) {
        fprintf(ps.out_, "\"%s\"", binary.data_); // TODO: must escape characters, is \0 always at the end?
      } else {
        //'samples, 'instructions, 'code, 'bits, 'mask, 'cbits etc.
        fprintf(ps.out_, "binary(");
        ps.expect_symbol(true);
        binary.class_.Print(ps);
        ps.expect_symbol(false);
        fprintf(ps.out_, ": <%ld bytes>)", size());
      }
      break;
    case Tag::large_binary:
      fprintf(ps.out_, "large_binary('");
      ps.expect_symbol(true);
      binary.class_.Print(ps);
      ps.expect_symbol(false);
      fprintf(ps.out_, ": <%ld bytes>)", size());
      break;
    case Tag::array:
      static_cast<const Array*>(this)->Print(ps);
      break;
    case Tag::frame:
      static_cast<const Frame*>(this)->Print(ps);
      break;
    case Tag::real:
      fprintf(ps.out_, "%g", real.value_);
      break;
    case Tag::symbol:
      if (!ps.symbol_expected())
        fprintf(ps.out_, "'");
      fprintf(ps.out_, "%s", symbol.string_);
      break;
    case Tag::native_ptr:
      fprintf(ps.out_, "<NativePtr>");
      break;
    case Tag::reserved:
      fprintf(ps.out_, "<Reserved>");
      break;
  }
  return 0;
}

// Flags can be 1 (kMapSorted), 2(kMapShared), 4 (kMapProto)
nos::Frame::Frame()
: SlottedObject( Frame_{ new Map(Ref(0), 1), new Ref[4], 4 }, 0)
{ }

Ref nos::AllocateFrame()
{
  return Ref(new nos::Frame());
}

void nos::SetFrameSlot(RefArg obj, RefArg tag, RefArg value)
{
  if (!obj.IsFrame())
    throw BadTypeWithFrameData(kNSErrNotAFrame);
  if (obj.IsReadOnly())
    throw FramesWithBadValue(kNSErrObjectReadOnly);
  if (!tag.IsSymbol())
    throw BadTypeWithFrameData(kNSErrNotASymbol);
  Frame *frame = static_cast<Frame*>(obj.GetObject());
  frame->SetSlot(tag, value);
}

nos::Array::Array(RefArg obj_class, Index length)
: SlottedObject( Array_{ obj_class, new Ref[length], 0 }, (uint32_t)length)
{ }

nos::Array::Array(RefArg obj_class)
: SlottedObject( Array_{ obj_class, new Ref[4], 4 }, 0)
{ }

nos::Map::Map(RefArg obj_class, Index length)
: Array(obj_class, length)
{ }

nos::Map::Map(RefArg obj_class)
: Array(obj_class)
{ }

Ref nos::AllocateArray(RefArg theClass, Index length)
{
  return Ref(new nos::Array(theClass, length));
}

Index nos::Array::AddSlot(RefArg value)
{
  Index len = Length();
  SetLength(len + 1);
  assert((len >= 0) && (len < (Index)(size_/sizeof(Ref))));
  array.slot_[len] = value;
  return len;
}

void nos::Frame::SetSlot(RefArg tag, RefArg value)
{
  // TODO: frame.map_->FIndOffset(tag);
  Index i = FindOffset(frame.map_, tag);
  if (i == -1) {
    i = frame.map_->AddSlot(tag);
    if (i == -1)
      return; // TODO: throw
    SetLength(i);
  }
  assert((i-1 >= 0) && (i-1 < (Index)(size_/sizeof(Ref))));
  frame.slot_[i-1] = value;
}

Index nos::FindOffset(Ref map_ref, Ref tag)
{
  if (!map_ref.IsArray())
    return -1; // TODO: throw
  Array *map = static_cast<Array*>(map_ref.GetObject());
  Index i = 1, n = map->Length(); // TODO: index[0] may point to a super map!
  for (;i<n;++i) {
    if (SymbolCompare(map->GetSlot(i), tag)==0)
      return i-1;
  }
  return -1;
}

//Index nos::Frame::AddSlot(RefArg tag)
//{
//  (void)tag;
//  return -1;
//}

bool nos::IsReadOnly(RefArg ref)
{
  if (IsPtr(ref)) {
    Object *obj = ref.GetObject();
    return obj->IsReadOnly();
  } else {
    return false;
  }
}

Index nos::AddArraySlot(RefArg array_ref, RefArg value)
{
  if (!array_ref.IsArray())
    throw BadTypeWithFrameData(kNSErrNotAnArray);
  if (IsReadOnly(array_ref))
    throw FramesWithBadValue(kNSErrObjectReadOnly);

  Array *array = static_cast<Array*>(array_ref.GetObject());
  return array->AddSlot(value);
}

//// TODO: this should probably throw an exception?
//Ref Ref::GetArraySlot(Index slot) const {
//  return IsArray() ? o->GetSlot(slot) : RefNIL;
//}
//

Ref nos::GetArraySlot(RefArg array_obj, Index slot)
{
  (void)array_obj;
  (void)slot;
  return RefNIL;
}

int nos::Array::Print(PrintState &ps) const
{
  fprintf(ps.out_, "[\n");
  ps.incr_depth();
  ps.tab();
  ps.expect_symbol(true);
  array.class_.Print(ps);
  ps.expect_symbol(false);
  fprintf(ps.out_, ":\n");
  int i, n = (int)(size()/sizeof(Ref));
  for (i=0; i<n; ++i) {
    ps.tab();
    array.slot_[i].Print(ps);
    if (i+1<n) fprintf(ps.out_, ",");
    fprintf(ps.out_, "\n");
  }
  ps.decr_depth();
  ps.tab();
  fprintf(ps.out_, "]");
  return 0;
}

int nos::Frame::Print(PrintState &ps) const
{
  fprintf(ps.out_, "{\n");
  ps.incr_depth();
  int i, n = (int)(size()/sizeof(Ref));
  for (i=0; i<n; ++i) {
    ps.tab();
    ps.expect_symbol(true);
    frame.map_->GetSlot(i+1).Print(ps);
    ps.expect_symbol(false);
    fprintf(ps.out_, ": ");
    GetSlot(i).Print(ps);
    if (i+1<n) fprintf(ps.out_, ",");
    fprintf(ps.out_, "\n");
  }
  ps.decr_depth();
  ps.tab();
  fprintf(ps.out_, "}");
  return 0;
}

Ref nos::MakeString(const char *str) {
  return Ref(new Object(str));
}

Ref nos::Sym(const char *name) {
  // TODO: check if the symbol already exists in the global symbols list
  // TODO: if it exists, return a Ref to the global symbol and return
  // TODO: if not, we must add it to the list
  // TODO: if list of known symbols is read-only, clone the list
  // TODO: return a Ref to the global symbol and return
  return Ref(new Symbol(name));
}

