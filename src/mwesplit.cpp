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


// TODO: We don't handle readings initiated by spaces (should we?)
const std::basic_regex<char> CG_LINE ("^"
				      "(\"<(.*)>\".*" // wordform, group 2
				      "|(\t+)(\"[^\"]*\"\\S*)(\\s+\\S+)*" // reading, group 3, 4, 5
				      ")");

const std::basic_regex<char> CG_WFTAG (".*\\s+\"<(.*)>\"");

struct Reading {
		std::string ana;
		std::string wftag;
};

struct Cohort {
		std::string form;
		std::vector<std::vector<Reading> > readings;
};

const std::string wftag(const std::string line)
{
	std::match_results<const char*> result;
	std::regex_match(line.c_str(), result, CG_WFTAG);
	if(!result.empty() && result[1].length() != 0) {
		return result[1];
	}
	else {
		return "";	// TODO: what if there's an empty wftag? call it a bug?
	}
}

const void print_reading(std::ostream& os, const Reading& r) {
	os << r.ana <<std::endl;
}

const void print_cohort(std::ostream& os, const Cohort& c) {
	os << "\"<" << c.form << ">\"" << std::endl;
	for(auto &r : c.readings) {
		for(auto &s : r) {
			print_reading(os, s);
		}
	}
}

const std::vector<Cohort> split_cohort(const Cohort& mwe) {
	std::vector<Cohort> cos;
	for(const auto& r : mwe.readings) {
		if(!r.empty() && r.front().wftag.empty()) {
			// head reading without wordform => not mwe (or buggy input), don't split
			return { mwe };
		}
		size_t pos = -1;
		for(const auto& s : r) {
			if(!s.wftag.empty()) {
				++pos;
				while(cos.size() < pos+1) {
					cos.push_back({ s.wftag, {} });
				}
				if(cos[pos].form != s.wftag) {
					std::cerr << "WARNING: Ambiguous word form tags for same cohort " << cos[pos].form << " vs " << s.wftag << std::endl;
				}
			}
			Reading n = { s.ana, "" };
			if(cos[pos].readings.empty()) {
				cos[pos].readings.push_back({n});
			}
			else {
				cos[pos].readings.back().push_back({n});
			}
		}
	}
	// The last word forms are the top readings:
	std::reverse(cos.begin(), cos.end());
	return cos;
}

const void split_and_print(std::ostream& os, const Cohort& c) {
	if(!c.form.empty()) {
		const std::vector<Cohort> cos = split_cohort(c);
		for(auto& c : cos) {
			print_cohort(os, c);
		}
	}
}


void run(std::istream& is, std::ostream& os)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
	std::ostringstream ss;
	Cohort cohort = { "", {} };
	size_t indentation = 0;
	for (std::string line; std::getline(is, line);) {
		std::match_results<const char*> result;
		std::regex_match(line.c_str(), result, CG_LINE);
		if(!result.empty() && result[2].length() != 0) {
			split_and_print(os, cohort);
			cohort = { result[2], {} };
			indentation = 0;
		}
		else if(!result.empty() && result[3].length() != 0) {
			auto rwf = wftag(line);
			Reading r = { line, rwf };
			if(cohort.readings.empty()) {
				cohort.readings.push_back({r});
			}
			else if((unsigned)result[3].length() > indentation) {
				// we know readings non-empty because above if:
				cohort.readings.back().push_back(r);
			}
			else {
				// indentation same or decreased (CG doesn't allow "ambiguous" subreadings
				// of a reading, so we always go all the way back up to main cohort here)
				cohort.readings.push_back({r});
			}
			indentation = result[3].length();
		}
		else {
			split_and_print(os, cohort);
			cohort = { "", {} };
			indentation = 0;
			os << line << std::endl;
		}
	}

	split_and_print(os, cohort);
}
}
