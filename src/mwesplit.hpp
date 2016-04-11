/*
* Copyright (C) 2016, Kevin Brubeck Unhammer <unhammer@fsfe.org>
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

#pragma once
#ifndef b3c2d0b8add5c9cb_MWESPLIT_H
#define b3c2d0b8add5c9cb_MWESPLIT_H


#include "util.hpp"

#include <locale>
#include <codecvt>

#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <unordered_map>
#include <exception>

namespace gtd {

typedef std::set<std::u16string> UStringSet;
typedef std::unordered_map<std::string, std::unordered_map<std::u16string, std::u16string> > msgmap;

enum LineType {
	WordformL, ReadingL, BlankL
};

void run(std::istream& is, std::ostream& os);


}

#endif
