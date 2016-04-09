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

#include "mwesplit.hpp"

namespace gtd {


const std::basic_regex<char> CG_LINE ("^"
				      "(\"<(.*)>\".*" // wordform, group 2
				      "|(\t)+(\"[^\"]*\"\\S*)(\\s+\\S+)*" // reading, group 3, 4, 5
				      ")");


struct Reading {
		std::string ana;
		std::string form;
		std::vector<Reading> subs;
};

struct Cohort {
		std::string form;
		std::vector<Reading> readings;
};

void run(std::istream& is, std::ostream& os)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
	std::ostringstream ss;
	for (std::string line; std::getline(is, line);) {
		os << line << std::endl;
		std::match_results<const char*> result;
		std::regex_match(line.c_str(), result, CG_LINE);
		if(!result.empty() && result[3].length() != 0) {
			os <<"reading"<<std::endl;
		}
	}
}
}
