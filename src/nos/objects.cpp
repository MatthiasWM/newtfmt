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

using namespace nos;


void Object::incr_ref_count() {
  ref_count_++;
}

void Object::decr_ref_count() {
  ref_count_--;
  // TODO: if ref count is 0, we can safely delete this object
}


//constexpr SymbolObject::SymbolObject(const char *) {
//}

//constexpr SymbolObject gSymArray{ "array" };


//constexpr SymbolObject gSymArray{ "array" };



//using RRef = std::variant<int, const int*>;
//
//constexpr int a = 3;
//constexpr int b = 4;
//
//constexpr RRef arr[] = { a, b };
//constexpr RRef arrptr[] = { &a, b };
//
//
//void xxx() {
//  (void)arr;
//  (void)arrptr;
//}
