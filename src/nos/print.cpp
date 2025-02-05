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

#include "nos/print.h"

#include "nos/types.h"
#include "nos/ref.h"

using namespace nos;

PrintState::PrintState(FILE *fout)
  : out_(fout)
{
}

PrintState::~PrintState()
{
}


/**
 Print any Ref.
 */
void nos::Print(RefArg p)
{
  PrintState state(stdout);
  p.Print(state);
}

